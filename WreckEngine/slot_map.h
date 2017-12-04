#pragma once

#include <vector>
#include <optional>

/*--------------------------------------------------------------------------------------------------------------------------
SLOT MAP
-------------
Container with contiguous storage for some type T (banded with uint32_ts) alongside strong index validity guarantees

  - Access to the container requires a key with an index into an indirection container and the "version" of the underlying object
  - That slot in the indirection layer contains another index for the actual data container and a version identifier to compare with the provided key
  - The slots remain constant, so the only way a key can be invalidated is if the item is removed from the container, changing its underlying version

  - The underlying data can have ordering, but slots have static locations disconnected from it
  - When inserting data, recycling unused slots is prioritized over allocating more
  - Iterating over the container will expose references to the internal data and the slots they are associated with
    - Use this interface for algorithms that mutate/reorder the container; anything else should use .values()
    - Reordering the container through the iterator interface will keep the slots up-to-date while ordering the underlying data appropriately
  - Reordering requires the underlying data to point to their slots
    - A non-intrusive implementation is impossible to implement efficiently
      - The working version iterates over the data, using its stored index to retrieve its slot, required for proper reordering
      - A version that iterates over the slots is possible, but the iteration order does not reflect the data and so can't work
      - The only non-intrusive method would be to search the slots for the correct index each time or use mixed hashing, which is significantly less efficient
----------------------------------------------------------------------------------------------------------------------------*/
template<typename T, template<typename...> typename Storage = std::vector>
class slot_map {
public:
    // the key returned to the user has the same version as the one in slots,
    // but the user index is the index in slots and the slots index is the one in data
    struct key { uint32_t index, version; };

private:
    struct internal_key {
        uint32_t index, version;

        internal_key() = default;
        internal_key(const internal_key&) = default;
        internal_key(internal_key&&) = default;
        internal_key& operator=(const internal_key& other) noexcept { index = other.index; return *this; }
        internal_key& operator=(internal_key&& other) noexcept { index = std::move(other.index); return *this; }

        friend bool operator==(const internal_key& a, decltype(a) b) { return std::tie(a.index, a.version) == std::tie(b.index, b.version); }
        friend bool operator!=(const internal_key& a, decltype(a) b) { return !(a == b); }
    };

    struct internal_data {
        T value;
        uint32_t slotIndex; // put after to respect alignment requirements of T

        internal_data(T val, uint32_t slot) : value(val), slotIndex(slot) {}
        template<typename... Args>
        internal_data(uint32_t slot, Args&&... args) : value(std::forward<Args>(args)...), slotIndex(slot) {}

        internal_data() = default;
        internal_data(const internal_data&) = default;
        internal_data(internal_data&&) = default;
        // copying from other internal_data implies value overwrite is desired
        internal_data& operator=(const internal_data& other) { value = other.value; return *this; }
        // moving into internal_data from another implies that they are swapping slots and the value moved out of will be moved into later
        internal_data& operator=(internal_data&& other) { using std::swap; swap(slotIndex, other.slotIndex); value = std::move(other.value); return *this; }

        template<typename U = internal_data> bool operator< (const U& other) const { return value < other.value; }
        template<typename U = internal_data> bool operator<=(const U& other) const { return value <= other.value; }
        template<typename U = internal_data> bool operator==(const U& other) const { return value == other.value; }
        template<typename U = internal_data> bool operator!=(const U& other) const { return value != other.value; }
        template<typename U = internal_data> bool operator> (const U& other) const { return value > other.value; }
        template<typename U = internal_data> bool operator>=(const U& other) const { return value >= other.value; }
    };

    Storage<internal_data> valueData;
    std::vector<internal_key> slots;
    std::vector<uint32_t> freeSlots;

    using storage_t = decltype(valueData);
    using internal_iter = decltype(std::begin(valueData));

    struct iter_value;

    // represents a "reordering" reference to data within the container and a pointer to its slot
    // this pointer allows discrepancies created by moving data around to be resolved
    // it's a pointer because if an iter_ref is move-assigned by another the values must update to reflect their new slots
    struct iter_ref {
        internal_data& data;
        internal_key* slot;
        const slot_map* container; // allows differentiation between other containers' iter_refs

        key get_key() const { return { data.slotIndex, slot->version }; }

        friend void swap(iter_ref& a, iter_ref& b) {
            using std::swap;
            swap(*a.slot, *b.slot);
            swap(a.slot, b.slot);
            swap(a.data, b.data);
            swap(a.container, b.container);
        }
        friend void swap(iter_ref& a, iter_value& b) { using std::swap; swap(a.data, b.data); }
        friend void swap(iter_value& a, iter_ref& b) { using std::swap; swap(a.data, b.data); }

        iter_ref(internal_data& _data, internal_key& _slot, const slot_map* _container) : data(_data), slot(&_slot), container(_container) {}

        iter_ref(const iter_ref& other) noexcept : data(other.data), slot(other.slot), container(other.container) {}
        //iter_ref(iter_ref&&) = delete; // move construction from another reference doesn't work normally
        iter_ref(iter_ref&& other) noexcept : data(other.data), slot(other.slot), container(other.container) {} // but it must because MSVC doesn't have guaranteed copy elision to prevent it from being called
        iter_ref& operator=(const iter_ref& other) {
            data = other.data;
            return *this;
        }
        iter_ref& operator=(iter_ref&& other) {
            using std::swap;
            if (container == other.container) {
                swap(*slot, *other.slot);
                swap(slot, other.slot); // this ensures that if the ref is stored, its slot remains accurate
            }
            data = std::move(other.data);
            return *this;
        }

        // no construction from a free value!
        iter_ref& operator=(const iter_value& val) noexcept {
            data = val.data;
            return *this;
        }
        iter_ref& operator=(iter_value&& val) noexcept {
            data = std::move(val.data);
            return *this;
        }

        operator internal_data&() { return data; }
        operator T&() { return data.value; }

        friend bool operator< (const iter_ref& a, const iter_ref& b) { return a.data < b.data; }
        friend bool operator<=(const iter_ref& a, const iter_ref& b) { return a.data <= b.data; }
        friend bool operator==(const iter_ref& a, const iter_ref& b) { return a.data == b.data; }
        friend bool operator!=(const iter_ref& a, const iter_ref& b) { return a.data != b.data; }
        friend bool operator> (const iter_ref& a, const iter_ref& b) { return a.data > b.data; }
        friend bool operator>=(const iter_ref& a, const iter_ref& b) { return a.data >= b.data; }

        friend bool operator< (const iter_ref& a, const iter_value& b) { return a.data < b.data; }
        friend bool operator<=(const iter_ref& a, const iter_value& b) { return a.data <= b.data; }
        friend bool operator==(const iter_ref& a, const iter_value& b) { return a.data == b.data; }
        friend bool operator!=(const iter_ref& a, const iter_value& b) { return a.data != b.data; }
        friend bool operator> (const iter_ref& a, const iter_value& b) { return a.data > b.data; }
        friend bool operator>=(const iter_ref& a, const iter_value& b) { return a.data >= b.data; }

        template<typename U> friend bool operator< (const iter_ref& a, const U& b) { return a.data.value < b; }
        template<typename U> friend bool operator<=(const iter_ref& a, const U& b) { return a.data.value <= b; }
        template<typename U> friend bool operator==(const iter_ref& a, const U& b) { return a.data.value == b; }
        template<typename U> friend bool operator!=(const iter_ref& a, const U& b) { return a.data.value != b; }
        template<typename U> friend bool operator> (const iter_ref& a, const U& b) { return a.data.value > b; }
        template<typename U> friend bool operator>=(const iter_ref& a, const U& b) { return a.data.value >= b; }

        template<typename U> friend bool operator< (const U& a, const iter_ref& b) { return a < b.data.value; }
        template<typename U> friend bool operator<=(const U& a, const iter_ref& b) { return a <= b.data.value; }
        template<typename U> friend bool operator==(const U& a, const iter_ref& b) { return a == b.data.value; }
        template<typename U> friend bool operator!=(const U& a, const iter_ref& b) { return a != b.data.value; }
        template<typename U> friend bool operator>=(const U& a, const iter_ref& b) { return a >= b.data.value; }
        template<typename U> friend bool operator> (const U& a, const iter_ref& b) { return a > b.data.value; }
    };

    // represents a free value of the container's data, used for moving values around
    // a free value's slot information is discarded since it's usually irrelevant 
    // swapping refs will move the hole around properly
    struct iter_value {
        internal_data data;

        iter_value(const iter_value&) = default;
        iter_value(iter_value&&) = default;
        iter_value& operator=(const iter_value&) = default;
        iter_value& operator=(iter_value&&) = default;

        iter_value(const iter_ref& ref) noexcept : data(ref.data) {}
        iter_value(iter_ref&& ref) noexcept : data(std::move(ref.data)) {}
        iter_value& operator=(const iter_ref& ref) noexcept {
            data = ref.data;
            return *this;
        }
        iter_value& operator=(iter_ref&& ref) noexcept {
            data = std::move(ref.data);
            return *this;
        }

        operator internal_data&() { return data; }
        operator T&() { return data.value; }

        friend bool operator< (const iter_value& a, const iter_value& b) { return a.data < b.data; }
        friend bool operator<=(const iter_value& a, const iter_value& b) { return a.data <= b.data; }
        friend bool operator==(const iter_value& a, const iter_value& b) { return a.data == b.data; }
        friend bool operator!=(const iter_value& a, const iter_value& b) { return a.data != b.data; }
        friend bool operator> (const iter_value& a, const iter_value& b) { return a.data > b.data; }
        friend bool operator>=(const iter_value& a, const iter_value& b) { return a.data >= b.data; }

        friend bool operator< (const iter_value& a, const iter_ref& b) { return a.data < b.data; }
        friend bool operator<=(const iter_value& a, const iter_ref& b) { return a.data <= b.data; }
        friend bool operator==(const iter_value& a, const iter_ref& b) { return a.data == b.data; }
        friend bool operator!=(const iter_value& a, const iter_ref& b) { return a.data != b.data; }
        friend bool operator> (const iter_value& a, const iter_ref& b) { return a.data > b.data; }
        friend bool operator>=(const iter_value& a, const iter_ref& b) { return a.data >= b.data; }

        template<typename U> friend bool operator< (const iter_value& a, const U& b) { return a.data.value < b; }
        template<typename U> friend bool operator<=(const iter_value& a, const U& b) { return a.data.value <= b; }
        template<typename U> friend bool operator==(const iter_value& a, const U& b) { return a.data.value == b; }
        template<typename U> friend bool operator!=(const iter_value& a, const U& b) { return a.data.value != b; }
        template<typename U> friend bool operator> (const iter_value& a, const U& b) { return a.data.value > b; }
        template<typename U> friend bool operator>=(const iter_value& a, const U& b) { return a.data.value >= b; }

        template<typename U> friend bool operator< (const U& a, const iter_value& b) { return a < b.data.value; }
        template<typename U> friend bool operator<=(const U& a, const iter_value& b) { return a <= b.data.value; }
        template<typename U> friend bool operator==(const U& a, const iter_value& b) { return a == b.data.value; }
        template<typename U> friend bool operator!=(const U& a, const iter_value& b) { return a != b.data.value; }
        template<typename U> friend bool operator>=(const U& a, const iter_value& b) { return a >= b.data.value; }
        template<typename U> friend bool operator> (const U& a, const iter_value& b) { return a > b.data.value; }
    };

    template<typename Ref_t>
    struct iterator_impl {
        using difference_type = std::ptrdiff_t;
        using value_type = iter_value;
        using pointer = std::conditional_t<std::is_const_v<Ref_t>, const T*, T*>;
        using reference = Ref_t;
        using iterator_category = std::random_access_iterator_tag;

        slot_map* container;
        int index;

        operator iterator_impl<const Ref_t>&() { return reinterpret_cast<iterator_impl<const Ref_t>&>(*this); }

        reference operator*() { auto& data = *data_addr(); return { data, container->slots[data.slotIndex], container }; }
        reference operator[](int diff) { return *(*this + diff); }

        // this probably makes the most sense for the potential usage
        pointer operator->() { return &data_addr()->value; }

        iterator_impl& operator++() { ++index; return *this; }
        iterator_impl operator++(int) { auto it = *this; ++*this; return it; }

        iterator_impl& operator--() { --index; return *this; }
        iterator_impl operator--(int) { auto it = *this; --index; return it; }

        iterator_impl& operator+=(int diff) { index += diff; return *this; }
        iterator_impl& operator-=(int diff) { return *this += -diff; }

        friend difference_type operator-(const iterator_impl& a, decltype(a) b) { return a.data_addr() - b.data_addr(); }

        friend iterator_impl operator+(iterator_impl it, int diff) { return it += diff; }
        friend iterator_impl operator+(int diff, iterator_impl it) { return it += diff; }
        friend iterator_impl operator-(iterator_impl it, int diff) { return it -= diff; }

        friend bool operator< (const iterator_impl& a, decltype(a) b) { return a.data_addr() < b.data_addr(); }
        friend bool operator<=(const iterator_impl& a, decltype(a) b) { return a.data_addr() <= b.data_addr(); }
        friend bool operator==(const iterator_impl& a, decltype(a) b) { return a.data_addr() == b.data_addr(); }
        friend bool operator!=(const iterator_impl& a, decltype(a) b) { return a.data_addr() != b.data_addr(); }
        friend bool operator> (const iterator_impl& a, decltype(a) b) { return a.data_addr() > b.data_addr(); }
        friend bool operator>=(const iterator_impl& a, decltype(a) b) { return a.data_addr() >= b.data_addr(); }
    private:
        auto* data_addr() const { return container->valueData.data() + index; }
    };
    friend iterator_impl;

    template<typename U>
    struct value_iter_impl {
        using value_type = T;
        using reference = U&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = typename internal_iter::iterator_category;

        std::conditional_t<std::is_const_v<U>, const internal_data*, internal_data*> ptr;

        operator value_iter_impl<const U>&() { return reinterpret_cast<value_iter_impl<const U>&(*this); }

        reference operator*() { return ptr->value; }
        reference operator[](int diff) { return (ptr + diff)->value; }

        value_iter_impl& operator++() { ++ptr; return *this; }
        value_iter_impl operator++(int) { auto it = *this; ++*this; return it; }

        value_iter_impl& operator--() { --ptr; return *this; }
        value_iter_impl operator--(int) { auto it = *this; --index; return it; }

        value_iter_impl& operator+=(int diff) { ptr += diff; return *this; }
        value_iter_impl& operator-=(int diff) { return *this += -diff; }

        friend difference_type operator-(const value_iter_impl& a, decltype(a) b) { return a.ptr - b.ptr; }

        friend value_iter_impl operator+(value_iter_impl it, int diff) { return it += diff; }
        friend value_iter_impl operator+(int diff, value_iter_impl it) { return it += diff; }
        friend value_iter_impl operator-(value_iter_impl it, int diff) { return it -= diff; }

        friend bool operator< (const value_iter_impl& a, decltype(a) b) { return a.ptr < b.ptr; }
        friend bool operator<=(const value_iter_impl& a, decltype(a) b) { return a.ptr <= b.ptr; }
        friend bool operator==(const value_iter_impl& a, decltype(a) b) { return a.ptr == b.ptr; }
        friend bool operator!=(const value_iter_impl& a, decltype(a) b) { return a.ptr != b.ptr; }
        friend bool operator> (const value_iter_impl& a, decltype(a) b) { return a.ptr > b.ptr; }
        friend bool operator>=(const value_iter_impl& a, decltype(a) b) { return a.ptr >= b.ptr; }
    };

    template<typename Val_storage>
    class value_iter {
    public:
        using iterator = std::conditional_t<std::is_const_v<Val_storage>, value_iter_impl<const T>, value_iter_impl<T>>;
        using const_iterator = value_iter_impl<const T>;

        value_iter(Val_storage& storage) : values(storage) {}

        std::conditional_t<std::is_const_v<Val_storage>, const T&, T&> operator[](size_t index) { return values[index].value; }
        const T& operator[](size_t index) const { return values[index].value; }

        iterator begin() { return { values.data() }; }
        iterator end()   { return begin() + values.size(); }

        const_iterator begin() const { return { values.data() }; }
        const_iterator end()   const { return begin() + values.size(); }

        std::reverse_iterator<iterator> rbegin() { return end(); }
        std::reverse_iterator<iterator> rend()   { return begin(); }

        std::reverse_iterator<const_iterator> rbegin() const { return end(); }
        std::reverse_iterator<const_iterator> rend()   const { return begin(); }
    private:
        Val_storage& values;
    };

    using value_iterator = value_iter<storage_t>;
    using const_value_iterator = value_iter<const storage_t>;

    void freeSlot(internal_key& slot, uint32_t index) {
        ++slot.version;
        freeSlots.push_back(index);
    }

    key acquireSlotForIndex(uint32_t index) {
        if (freeSlots.size()) {
            auto loc = freeSlots.back();
            freeSlots.pop_back();

            auto& slot = slots[loc];
            slot.index = index;
            return { loc, slot.version };
        }
        else {
            key k{ slots.size(), 0 };
            slots.push_back({ index, 0 });
            return k;
        }
    }
public:
    slot_map() = default;
    slot_map(size_t size) {
        while (size-- > 0) emplace_back();
    }
    slot_map(std::initializer_list<T> ilist) {
        valueData.reserve(ilist.size());
        slots.reserve(ilist.size() + 1);
        for (auto& item : ilist) {
            push_back(item);
        }
    }

    slot_map(const slot_map&) = default;
    slot_map(slot_map&&) = default;

    // copy assignment only duplicates values, invalidating old slots
    // this means an assigned slot_map can't reuse keys from the other slot_map
    slot_map& operator=(const slot_map& other) {
        clear(); 
        valueData.reserve(other.size());
        slots.reserve(other.size() + 1);
        for (auto& val : other.values()) 
            emplace_back(val);
        return *this;
    }

    // move assignment invalidates old slots, but can use slots from other
    slot_map& operator=(slot_map&& other) = default;

    slot_map& operator=(std::initializer_list<T> ilist) {
        clear();
        valueData.reserve(ilist.size());
        slots.reserve(ilist.size() + 1);
        for (auto& item : ilist) 
            push_back(item);
        return *this;
    }

    // this iterator is meant for reordering operations, as swapping dereferenced iterators will correctly update the slots along with the data
    // do not attempt to reorder the container in any other way; do not use this iterator for value operations, as it's less efficient
    // to accommodate reordering, the value_type will initialize references on construction, and assign them on assignment
    // this may lead to strange behavior for copy-required operations
    using iterator = iterator_impl<iter_ref>;
    using const_iterator = iterator_impl<const iter_ref>;

    // returns an iterator for reordering the container
    // for simple iteration, go through .values()
    iterator begin() { return { this, 0 }; }
    iterator end() { return { this, (int)valueData.size() }; }

    const_iterator begin() const { return { this, 0 }; }
    const_iterator end() const { return { this, (int)valueData.size() }; }

    // returns a helper struct for simple iteration through the data
    // for reordering, go through the container's own iterators
    value_iterator values() { return value_iterator(valueData); }
    const_value_iterator values() const { return const_value_iterator(valueData); }

    static constexpr size_t dataSize = sizeof(internal_data);

    // returns the address of the backing data buffer
    internal_data* data() { return valueData.data(); }
    const internal_data* data() const { return valueData.data(); }

    size_t size() const { return valueData.size(); }

    std::optional<key> key_of_index(uint32_t dataIndex) const {
        if (dataIndex >= valueData.size()) return std::nullopt;
        auto slotIndex = valueData[dataIndex].slotIndex;
        return key{ slotIndex, slots[slotIndex].version };
    }

    uint32_t index_of_key(key k) {
        auto slot = slots[k.index];
        return (slot.version == k.version) ? slot.index : valueData.size();
    }

    T& front() { return valueData.front().value; }
    const T& front() const { return valueData.front().value; }

    T& back() { return valueData.back().value; }
    const T& back() const { return valueData.back().value; }

    T* try_get(key k) {
        auto slot = slots[k.index];
        return (slot.version == k.version) ? &valueData[slot.index].value : nullptr;
    }

    const T* try_get(key k) const {
        auto slot = slots[k.index];
        return (slot.version == k.version) ? &valueData[slot.index].value : nullptr;
    }

    T& at(key k) { return *try_get(k); }
    T& operator[](key k) { return at(k); }

    const T& at(key k) const { return *try_get(k); }
    const T& operator[](key k) const { return at(k); }

    // use for direct indexed access into the underlying container
    T& valueAt(size_t index) { return valueData[index].value; }
    const T& valueAt(size_t index) const { return valueData[index].value; }

    void clear() {
        valueData.clear();
        freeSlots.reserve(slots.size());
        uint32_t count = 0;
        for (auto& slot : slots) freeSlot(slot, count++);
    }

    void reserve(size_t capacity) {
        valueData.reserve(capacity);
        slots.reserve(capacity - freeSlots.size());
    }

    // returns the indirect key to the underlying data
    key push_back(const T& val) {
        auto userKey = acquireSlotForIndex(valueData.size());
        valueData.push_back({ val, userKey.index });
        return userKey;
    }

    // returns the indirect key to the underlying data
    template<typename... Args>
    key emplace_back(Args&&... args) {
        auto userKey = acquireSlotForIndex(valueData.size());
        valueData.emplace_back(userKey.index, std::forward<Args>(args)...);
        return userKey;
    }

    key insert(const_iterator pos, const T& val) {
        // update slots after the insert location
        auto index = pos - begin();
        std::for_each(pos, (const_iterator)end(), [](auto val) { ++val.slot->index; });

        // acquire a slot and insert
        auto userKey = acquireSlotForIndex(index);
        valueData.insert(std::begin(valueData) + index, internal_data{ val, userKey.index });
        return userKey;
    }

    template<typename InputIter>
    void insert(const_iterator pos, InputIter first, InputIter last) {
        if (last <= first) return;

        // increment slot indices after the insert
        auto startIndex = pos - begin(), diff = last - first, insertEnd = startIndex + diff;
        std::for_each(pos, (const_iterator)end(), [diff](auto val) { val.slot->index += diff; });

        // insert more elements to be moved into
        auto oldSize = size();
        reserve(oldSize + diff);
        for (decltype(diff) i = 0; i < diff; ++i) valueData.emplace_back();

        // move the conflicting elements down
        auto valBeg = std::begin(valueData);
        std::move_backward(valBeg + startIndex, valBeg + oldSize, std::end(valueData));

        // acquire slots and insert one at a time
        for (auto i = startIndex; i < insertEnd; ++i) {
            auto key = acquireSlotForIndex(i);

            auto& data = valueData[i];
            data.slotIndex = key.index;
            data.value = *first++;
        }
    }

    template<typename... Args>
    key emplace(const_iterator pos, Args&&... args) {
        // update slots after the insert location
        auto index = pos - begin();
        std::for_each(pos, (const_iterator)end(), [](auto val) { ++val.slot->index; });

        // acquire a slot and insert
        auto userKey = acquireSlotForIndex(index);
        valueData.emplace(std::begin(valueData) + index, userKey.index, std::forward<Args>(args)...);
        return userKey;
    }

    void pop_back() {
        if (valueData.empty()) return;

        auto index = valueData.back().slotIndex;
        valueData.pop_back();
        freeSlot(slots[index], index);
    }

    void removeAtBack(key k) {
        auto& slot = slots[k.index];
        if (slot.version != k.version) return;

        auto loc = begin() + slot.index;
        auto last = end() - 1;

        freeSlot(slot, k.index);
        std::iter_swap(loc, last);
        valueData.pop_back();
    }

    void remove(key k) {
        auto& slot = slots[k.index];
        if (slot.version != k.version) return;

        erase(begin() + slot.index);
    }

    void erase(const_iterator pos) {
        auto info = *pos;
        auto& slot = *info.slot;

        freeSlot(slot, info.data.slotIndex);
        valueData.erase(std::begin(valueData) + slot.index);

        std::for_each(begin() + slot.index, end(), [](auto val) { --val.slot->index; }); // decrement all the slots so they're up to date
    }

    void erase(const_iterator first, const_iterator last) {
        if (last <= first) return;

        auto diff = last - first;
        auto index = first.index;

        auto eraseBeg = std::begin(valueData) + index, eraseEnd = std::begin(valueData) + last.index;
        std::for_each(first, --last, [this](auto val) { freeSlot(*val.slot, val.data.slotIndex); });
        valueData.erase(eraseBeg, eraseEnd);

        std::for_each(begin() + index, end(), [diff](auto val) { val.slot->index -= diff; });
    }

    void erase(typename value_iterator::const_iterator first, decltype(first) last) {
        if (first.container != this || first.container != last.container) return;
        auto valBeg = values().begin();
        erase(begin() + (first - valBeg), begin() + (last - valBeg));
    }
};