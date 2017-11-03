#pragma once

#include <type_traits>

/*
----------------------------------------------------------
PROPERTIES/ACCESSORS - a C++ experiment

C++ does not natively support properties a la C#; this is unfortunate because they're very convenient.
Using some sneaky offset tricks (which could be done at compile time IF OFFSETOF WORKED PROPERLY) 
and macros to reduce the boilerplate, I've approached a close-ish approximation of properties.

These properties will always call their set on assignment, with overloads of the modifier op=s (e.g. +=) 
enabled when the property is get-set and the operators are overloaded for the types.
The get is executed through operator(), with an optional implicit cast operator enabled through WR_PROP_IMPLICIT_CAST.
The implicit cast will happen when passed to parameters that explicitly call for that type, or when being assigned to something of that type.
They will NOT be implicitly cast for template specializations (unfortunately) or when accessing fields of the underlying type.
You can explicitly cast in these cases (only when the conversion is enabled) or just use operator() as normal.

To declare a property "properly", there are a variety of annoying steps, including making the property public while the field it wraps and
its getter and/or setter are private. That's where the macros come in:
  - PROP_G = property with get only
  - PROP_S = property with set only
  - PROP_GS = property with get and set

Pass in the access level, containing class, wrapped type, desired property name, and then the body of the get and/or set wrapped in braces and all the necessary code
will be auto-generated. A few notes:
  - Modified assignments, e.g. +=, will just call set on the current get modified by the new value
  - The setter returns a & to the property itself for consistency with normal C++ op= semantics and to prevent accidental incorrect gets.

The overhead for properties is fairly minimal, all things considered. 
  - They are 8/16 bytes each, (sizeof(void*) + sizeof(size_t) on x86/64) 
  - Each call to get/set goes through a baked function pointer 
    - Because it's known at compile-time, it's probably inlined and doesn't go through a function pointer at all.
  - At construction, properties simply take a pointer to their container, and calculate their offset within that container and store it.
    - These values are const and never change after initialization.
  - The property types are copyable and movable.
    - Assignment for properties is a no-op (the containing ptr and offset don't need to be updated)
      - It could be argued that get-set properties should call set with the other's get
      - This would allow assignment chains for get-set properties and lead to easier syntax for those cases
      - There would be overhead and redundancy when copying/moving into the containing object if this were the case
      - Maybe make this opt-in?
    - Copy and move construction simply calculate a new container pointer based on the offset of the other property and their own address.
  - operator() is preferred over implicit casting in my opinion as it makes it a lot more clear that there is potential work happening.
This leaves only a minor pointer access redundancy (the compiler can't identify the internal pointer and the one it's retrieving from are the same)
which is very minimal. 
The offset size and minor copy/move overhead could be reduced as it's theoretically known at compile-time, 
but offset is only defined for standard-layout objects as dictated by the standard.
The copy/move overhead is unavoidable, but very minimal.


While properties in this form are [mostly] convenient, there's always more than one way to skin a cat. Efficient "properties" in C++ are often implemented as:
  - public const& to a private field (get only)
  - public & to a private field (get and set)
  * set only cannot be represented via a &
  - accessor methods for private fields

The first 3 bullets are easy enough to deal with, (though they delete all assignment, copy, and move operations) but the last becomes cumbersome
quickly with enough fields. To respond to this, there are similar, more efficient macros for accessors as well:
  - ACCS_G = get only accessor
  - ACCS_S = set only accessor
  - ACCS_GS = get and set accessors

By default, these can simply be passed the type and name for the accessor(s) and it will generate the basic get [type name() { return _name; }] and
set [void name(type value) { _name = value; }], but sometimes something more sophisticated is necessary. This can be facilitated via the use of these qualifiers,
which can be chained in the provided order:
  1. _T = custom type for accessor(s). G and S take one type to replace the return type and passed type respectively, but GS takes both
  2. _C = custom body for accessor(s). G and S only require their specific one, but GS requires both

General notes for both properties and accessors:
  - The internal field will always be created as _name. The class can use this internally for efficiency.
  - Properties and accessors will ALWAYS be public (there wouldn't be much point otherwise)
  - The access parameter should ideally correspond to the actual access level containing the macro.
    - This is because the macro will change the access level below it in the class definition to whatever is passed.
  - The macros must be followed by a semicolon. The last statement of any of the macros declares the internal field.
    - Because of this, it can either be left uninitialized/default constructed (macro(...);) or be given a default value. (macro(...) = something;/macro(...){something})
  - Inside the setter, the passed value is called 'value' (like C#) and is referred to as such
  - As a general rule, parameters are passed in this order: access level, type(s), name, body/bodies

Future developments:
  - If C++ ever gets around to adding operator., then properties could access their underlying fields without outward-facing casts, which would be nice.
    - This functionality can almost be implemented for pointer types via the dereference and -> operators, but disabling them when irrelevant is difficult.
  - A major detraction for properties is the inability to implicitly cast for template specializations, most glaringly operators.
    - operator. could help when the property is on the LHS, though not the RHS.
  - C++20 has gotten reflection and metaclass proposals, which could make properties truly zero-overhead and a lot more succinct.
    Metaclasses actually use properties as a prime example! Of course, that would make this technique obsolete.
----------------------------------------------------------
*/

#define DECL_FLD(type, name) type _ ## name

#define DEF_GET_NAME(name) get_ ## name
#define DEF_SET_NAME(name) set_ ## name

#define DECL_DETAIL_NAME(name) prop_ ## name ## _impl
#define DECL_DETAIL(clss, type, name, get, set) struct DECL_DETAIL_NAME(name) { \
using parent_t = clss; \
using storage_t = type; \
static constexpr auto get_func = get; \
static constexpr auto set_func = set; \
};

#define DEF_GET_P(type, name, body) inline type DEF_GET_NAME(name)() const body;
#define DEF_GET_C(t_get, name, body) inline t_get name() const body;
#define DEF_GET(t_get, name) DEF_GET_C(t_get, name, { return _ ## name; });

#define DEF_SET_P(type, name, body) inline type const& DEF_SET_NAME(name)(type value) body;
#define DEF_SET_C(t_set, name, body) inline void name(t_set value) body;
#define DEF_SET(t_set, name) DEF_SET_C(t_set, name, { _ ## name = value; });

#define DEF_PROP_G(clss, type, access, name)  private: DECL_DETAIL(clss, type, name, &clss::DEF_GET_NAME(name), nullptr) public: property<DECL_DETAIL_NAME(name), access> name{this};
#define DEF_PROP_S(clss, type, access, name)  private: DECL_DETAIL(clss, type, name, nullptr, &clss::DEF_SET_NAME(name)) public: property<DECL_DETAIL_NAME(name), access> name{this};
#define DEF_PROP_GS(clss, type, access, name) private: DECL_DETAIL(clss, type, name, &clss::DEF_GET_NAME(name), &clss::DEF_SET_NAME(name)) public: property<DECL_DETAIL_NAME(name), access> name{this};

#define PROP_G(access, clss, type, name, get_body) access: DEF_GET_P(type, name, get_body) DEF_PROP_G(clss, type, property_type::GET, name) access: DECL_FLD(type, name)
#define PROP_S(access, clss, type, name, set_body) access: DEF_SET_P(type, name, set_body) DEF_PROP_S(clss, type, property_type::SET, name) access: DECL_FLD(type, name)
#define PROP_GS(access, clss, type, name, get_body, set_body) access: DEF_GET_P(type, name, get_body) DEF_SET_P(type, name, set_body) DEF_PROP_GS(clss, type, property_type::GET_SET, name) access: DECL_FLD(type, name)

#define ACCS_G(access, type, name)         public: DEF_GET(type, name)         access: DECL_FLD(type, name)
#define ACCS_G_C(access, type, name, body) public: DEF_GET_C(type, name, body) access: DECL_FLD(type, name)
#define ACCS_G_T(access, type, t_get, name)         public: DEF_GET(t_get, name)         access: DECL_FLD(type, name)
#define ACCS_G_T_C(access, type, t_get, name, body) public: DEF_GET_C(t_get, name, body) access: DECL_FLD(type, name)

#define ACCS_S(access, type, name)         public: DEF_SET(type, name)         access: DECL_FLD(type, name)
#define ACCS_S_C(access, type, name, body) public: DEF_SET_C(type, name, body) access: DECL_FLD(type, name)
#define ACCS_S_T(access, type, t_set, name)         public: DEF_SET(t_set, name)         access: DECL_FLD(type, name)
#define ACCS_S_T_C(access, type, t_set, name, body) public: DEF_SET_C(t_set, name, body) access: DECL_FLD(type, name)

#define ACCS_GS(access, type, name)                       public: DEF_GET(type, name)             DEF_SET(type, name)             access: DECL_FLD(type, name)
#define ACCS_GS_C(access, type, name, get_body, set_body) public: DEF_GET_C(type, name, get_body) DEF_SET_C(type, name, set_body) access: DECL_FLD(type, name)
#define ACCS_GS_T(access, type, t_get, t_set, name)                       public: DEF_GET(t_get, name)             DEF_SET(t_set, name)             access: DECL_FLD(type, name)
#define ACCS_GS_T_C(access, type, t_get, t_set, name, get_body, set_body) public: DEF_GET_C(t_get, name, get_body) DEF_SET_C(t_set, name, set_body) access: DECL_FLD(type, name)

enum class property_type { GET = 1, SET = 2, GET_SET = 3 };

namespace detail {
    template<typename P>
    static inline P* get_parent_ptr(void* prop, size_t offsetInObj) {
        return (P*)((char*)(prop)-offsetInObj);
    }

    static inline size_t get_offset(void* parent, void* member) {
        return (size_t)((char*)member - (char*)parent);
    }

    // simple type that maintains a pointer to its containing object
    // meant to be inherited by more complex types that require this behavior
    // the condition is that it must be inherited first!!! otherwise the behavior will break and you'll get memory corruption
    template<typename P>
    struct parent_ptr {
        parent_ptr(P* _parent) : parent(_parent), offsetInParent(detail::get_offset(parent, this)) {}
        parent_ptr(const parent_ptr<P>& other) : parent(detail::get_parent_ptr<P>(this, other.offsetInParent)), offsetInParent(other.offsetInParent) {}
        parent_ptr(parent_ptr<P>&& other) : parent(detail::get_parent_ptr<P>(this, other.offsetInParent)), offsetInParent(other.offsetInParent) {}
        parent_ptr& operator=(const parent_ptr<P>& other) { return *this; }
        parent_ptr& operator=(parent_ptr<P>&& other) { return *this; }
    protected:
        P* const parent;
    private:
        const size_t offsetInParent;
    };

    template<property_type Access> using can_get = std::enable_if_t<(int)Access & (int)property_type::GET>;
    template<property_type Access> using can_set = std::enable_if_t<(int)Access & (int)property_type::SET>;
    template<property_type Access> using can_get_set = std::enable_if_t<Access == property_type::GET_SET>;
}

//#define WR_PROP_IMPLICIT_CAST

template<class PropInfo, property_type Access>
class property : public detail::parent_ptr<typename PropInfo::parent_t> {
    using P = typename PropInfo::parent_t;
    using T = typename PropInfo::storage_t;
    template<typename = detail::can_get<Access>> inline decltype(auto) get() const { return (parent->*PropInfo::get_func)(); }

    friend P; // ideally, property assignment would be private except to P's own assignment operators, but the syntax is limited
    property(P* _parent) : parent_ptr(_parent) {}
    property(const property<PropInfo, Access>&) = default;
    property(property<PropInfo, Access>&&) = default;

    template<typename other_t> property& op_eq_impl(other_t other) {
        parent_ptr::operator=(other);
        if constexpr(Access == property_type::GET_SET)
            return *this = other();
        else
            return *this;
    }
public:
    property& operator=(const property<PropInfo, Access>& other) { return op_eq_impl(other); }
    property& operator=(property<PropInfo, Access>&& other) { return op_eq_impl(other); }

    template<typename = detail::can_set<Access>> inline decltype(auto) operator=(T value) { (parent->*PropInfo::set_func)(value); return *this; }

    template<typename = detail::can_get<Access>> inline decltype(auto) operator()() const { return get(); }
#if defined(WR_PROP_IMPLICIT_CAST)
    template<typename = detail::can_get<Access>> inline operator T() const { return get(); }
#endif

    // there should be conditional overloads of *, ->, and [] for get, but I can't figure out how to apply SFINAE to them

    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator+=(const V& value) { return *this = get() + value; }
    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator-=(const V& value) { return *this = get() - value; }
    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator*=(const V& value) { return *this = get() * value; }
    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator/=(const V& value) { return *this = get() / value; }
    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator&=(const V& value) { return *this = get() & value; }
    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator|=(const V& value) { return *this = get() | value; }
    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator^=(const V& value) { return *this = get() ^ value; }
    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator>>=(const V& value) { return *this = get() >> value; }
    template<typename = detail::can_get_set<Access>, typename V> inline decltype(auto) operator<<=(const V& value) { return *this = get() << value; }
};

#undef WR_PROP_IMPLICIT_CAST