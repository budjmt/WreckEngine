#pragma once

/*------------------------------------------
PROXY POINTERS

  - A smart pointer that references data within some object while providing more robust 
    guarantees on validity than a regular reference. 
      - These guarantees differ with the backing container.
      - All proxies are invalidated if they outlive their backing container or their data is removed from the container.
      - Buffer types are invalidated when data is inserted or removed before the proxy data, or if the buffer is reordered.
      - Map-like and single-object types are never otherwise invalidated.
      - Proxies are not invalidated if the underlying memory is reallocated or moved.
  
  - Proxies are implemented via passing key data to a get function called using a pointer to the container.
    - This means retrieval is exactly as efficient as retrieving from the container itself.

  - The space used by a proxy is equivalent to a pointer + the size of the key data.
  - Proxies can be cast to any proxy with the same key data tuple type.
  - Pointer arithmetic cannot be applied directly to a proxy, get a pointer after retrieving the data and operate on that.

------------------------------------------*/

namespace detail {
    template<typename R, typename T, typename... Args> std::tuple<Args...> args_as_tuple(R(T::*)(Args...)) { return std::tuple<Args...>(); }
    template<typename R, typename T, typename... Args> std::tuple<Args...> args_as_tuple(R(T::*)(Args...) const) { return std::tuple<Args...>(); }
    template<typename R, typename... Args> std::tuple<Args...> args_as_tuple(R(*)(Args...)) { return std::tuple<Args...>(); }

    template<typename R, typename T, typename... Args> constexpr decltype(auto) get_pure_func(R(T::*foo)(Args...)) { return foo; }
    template<typename R, typename T, typename... Args> constexpr decltype(auto) get_const_func(R(T::*foo)(Args...) const) { return foo; }

    template<typename T, typename Tuple> struct add_first_type {};
    template<typename T, typename... Ts> struct add_first_type<T, std::tuple<Ts...>> { using type = std::tuple<T, Ts...>; };
    template<typename T, typename Tuple> using add_first_type_t = typename add_first_type<T, Tuple>::type;

    template<typename T> struct data_tuple {};
    template<typename... T> struct data_tuple<std::tuple<T...>> { using type = std::tuple<std::remove_reference_t<std::remove_cv_t<T>>...>; };
    template<typename T> using data_tuple_t = typename data_tuple<T>::type;

    // can replace with template var when MSVC IS COMPLIANT
    template<typename Container>
    struct proxy_get {
        static constexpr auto func = [] {
            if constexpr(std::is_const_v<Container>)
                return get_const_func(&Container::at);
            else
                return get_pure_func(&Container::at);
        }();
    };
}

template<typename Container>
struct proxy_ptr {
    proxy_ptr() = default; // this is just to enable default constructors up the chain; don't actually use this
    template<typename... Args> proxy_ptr(Container& owner, Args&&... getArgs) : getParams{ &owner, std::forward<Args>(getArgs)... } {}
    proxy_ptr(const proxy_ptr& other) = default;
    proxy_ptr(proxy_ptr&& other) = default;
    proxy_ptr& operator=(const proxy_ptr& other) = default;
    proxy_ptr& operator=(proxy_ptr&& other) = default;

    auto& operator*() const { return get(); }
    auto* operator->() const { return &get(); }

    template<typename Other, typename = std::enable_if_t<std::is_same_v<proxy_ptr<Container>::getArgs_t, proxy_ptr<Other>::getArgs_t>>>
    operator proxy_ptr<Other>&() { return reinterpret_cast<proxy_ptr<Other>&>(*this); }

    operator bool() { return owner(); }
    Container* owner() const { return std::get<0>(getParams); }
private:
    auto& get() const { return std::apply(detail::proxy_get<Container>::func, getParams); }

    using getArgs_t = detail::data_tuple_t<decltype(detail::args_as_tuple(detail::proxy_get<Container>::func))>;
    using tuple_t = detail::add_first_type_t<Container*, getArgs_t>;
    tuple_t getParams{};
};

template<typename Container> using proxy = proxy_ptr<Container>;

template<typename Container, typename... Args> proxy<Container> make_proxy(Container& c, Args&&... args) { return proxy<Container> { c, std::forward<Args>(args)... }; }