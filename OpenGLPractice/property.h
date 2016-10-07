#pragma once

/*
----------------------------------------------------------
PROPERTIES/ACCESSORS - a weird C++ experiment

C++ does not naturally support properties a la C#; this is unfortunate because they're very convenient.
So using a series of weird tricks and adapting an existing solution found at http://www.codeguru.com/cpp/cpp/cpp_mfc/article.php/c4031/Implementing-a-Property-in-C.htm
with some additional tricks with macros and template specialization, I've sort of kind of approached properties!

These properties will always call their set on assignment, and implicitly call their get when they're cast to their underlying types. 
They'll be cast when parameters they're passed to explicitly call for that type, or when being assigned to something of that type. 
They will NOT be implicitly cast for template specializations (unfortunately) or when accessing fields of the underlying type. 
You can explicitly cast, or call the object's operator() which has the same effect.

To declare a property "properly", there are a variety of annoying steps, including making the property public while the field it wraps and 
its getter and/or setter are private. That's where the macros come in: 
	- PROP_G = property with get only
	- PROP_S = property with set only
	- PROP_GS = property with get and set

Pass in the access level, containing class, wrapped type, desired property name, and then the body of the get and/or set wrapped in curly braces and all the necessary code 
will be auto-generated. A few notes: 
	- The property, if it has both get and set, can use all of the modified assignments, e.g. +=, 
	  which will just set the value to result of the operator on the current get and the new value
	- The setter must return a const& to the wrapped type. This can usually be accomplished by returning the internal assignment.


While properties in this form are [mostly] convenient, there's always more than one way to skin a cat. Efficient "properties" in C++ are often implemented as:
	- public const& to a private field (get only)
	- public & to a private field (get and set)
	* set only cannot be represented via a &
	- accessor methods for private fields

The first 3 bullets are easy enough to deal with, (though they delete the default operator=, if that was needed) but the last becomes cumbersome 
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
	- The wrapped field will always be created as _name. You can refer to this in a private context for efficiency.
	- The properties and accessors will ALWAYS be public
	- The access parameter should ideally correspond to the actual access level containing the macro. 
	  Otherwise, the macro will have the side effect of changing the current access level in the class definition to whatever is passed.
	- The macros must be followed by a semicolon. Since the last statement of any of the macros declares the private field, 
	  it can either be left uninitialized (macro(...);) or be given a default value. (macro(...) = something;)
	- Inside the setter, the passed value is called 'value' (like C#) and can be referred to as such
	- As a general rule, parameters are passed in this order: access level, type(s), name, body/bodies

Future developments:
	- If C++ ever gets around to adding operator., then properties could access their underlying fields without outward-facing casts, which would be nice.
	  This functionality is implemented for pointer types via the dereference and -> operators.
	- The only major detraction for properties remaining is the inability to implicitly cast for template specializations, most glaringly operators. 
	  If there was a good way to get this to work, that would be great. (operator. might help with this in cases where the property is on the LHS, 
	  but the RHS case is still annoying)
----------------------------------------------------------
*/

#define DECL_FLD(type, name) type _ ## name

#define DEF_PROP_G(clss, type, access, name)  property<clss, type, access> name = property<clss, type, access>(this, &clss::get_ ## name);
#define DEF_PROP_S(clss, type, access, name)  property<clss, type, access> name = property<clss, type, access>(this, &clss::set_ ## name);
#define DEF_PROP_GS(clss, type, access, name) property<clss, type, access> name = property<clss, type, access>(this, &clss::get_ ## name, &clss::set_ ## name);

#define DEF_GET_P(type, name, body) type get_ ## name() const body;
#define DEF_GET(t_get, name) DEF_GET_C(t_get, name, { return _ ## name; });
#define DEF_GET_C(t_get, name, body) inline t_get name() const body;

#define DEF_SET_P(type, name, body) const type& set_ ## name(type value) body;
#define DEF_SET(t_set, name) DEF_SET_C(t_set, name, { _ ## name = value; });
#define DEF_SET_C(t_set, name, body) inline void name(t_set value) body;

#define PROP_G(access, clss, type, name, get_body) public: DEF_PROP_G(clss, type, GET, name) access: DEF_GET_P(type, name, get_body) DECL_FLD(type, name)
#define PROP_S(access, clss, type, name, set_body) public: DEF_PROP_S(clss, type, SET, name) access: DEF_SET_P(type, name, set_body) DECL_FLD(type, name)
#define PROP_GS(access, clss, type, name, get_body, set_body) public: DEF_PROP_GS(clss, type, GET_SET, name) access: DEF_GET_P(type, name, get_body) DEF_SET_P(type, name, set_body) DECL_FLD(type, name)

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

enum property_type { GET = 1, SET = 2, GET_SET = 3 };
template <typename P, typename T, property_type pt> class property { };

template<class P, class T>
struct prop_g {
	prop_g(P* parent, T(P::*getter)() const) : g_parent(parent), get(getter) { }
	inline T operator()() const { return g(); }
	inline operator T() const { return g(); }
protected:
	P* g_parent = nullptr;
	T(P::*get)() const = nullptr;
	inline T g() const { return (g_parent->*get)(); }
};

template<class P, class T>
struct prop_g<P, T*> {
	prop_g(P* parent, T(P::*getter)() const) : g_parent(parent), get(getter) { }
	inline T* operator()() const { return g(); }
	inline operator T*() const { return g(); }
	inline T operator*() const { return *g(); }
	inline T* operator->() const { return g(); }
	inline T operator[](const size_t i) { return *(g() + i); }
	inline T* operator+(const int i) { return g() + i; }
	inline T* operator-(const int i) { return g() - i; }
protected:
	P* g_parent = nullptr;
	T*(P::*get)() const = nullptr;
	inline T* g() const { return (m_parent->*get)(); }
};

template<class P, class T>
struct prop_s {
	prop_s(P* parent, const T&(P::*setter)(T)) : s_parent(parent), set(setter) {}
	inline const T& operator=(T value) { return (s_parent->*set)(value); }
protected:
	P* s_parent = nullptr;
	const T&(P::*set)(T) = nullptr;
};

// it might be worth considering a specialization for objects that returns a const&
template<class P, class T>
struct property<P, T, GET> : prop_g<P, T> {
	property<P, T, GET>(P* parent, T(P::*getter)() const) : prop_g(parent, getter) { }
};

template<class P, class T>
struct property<P, T, SET> : prop_s<P, T> {
	property<P, T, SET>(P* parent, const T&(P::*setter)(T)) : prop_s(parent, setter) { }
	inline const T& operator=(T value) { return (s_parent->*set)(value); }
};

template<class P, class T>
struct property<P, T, GET_SET> : prop_g<P, T>, prop_s<P, T> {
	property<P, T, GET_SET>(P* parent, T(P::*getter)() const, const T&(P::*setter)(T)) : prop_g(parent, getter), prop_s(parent, setter) { }
	inline const T& operator=(T value) { return (s_parent->*set)(value); }
	inline const T& operator+=(const T value) { return operator=(g() + value); }
	inline const T& operator-=(const T value) { return operator=(g() - value); }
	inline const T& operator*=(const T value) { return operator=(g() * value); }
	inline const T& operator/=(const T value) { return operator=(g() / value); }
	inline const T& operator&=(const T value) { return operator=(g() & value); }
	inline const T& operator|=(const T value) { return operator=(g() | value); }
	inline const T& operator^=(const T value) { return operator=(g() ^ value); }
	inline const T& operator>>=(const size_t value) { return operator=(g() >> value); }
	inline const T& operator<<=(const size_t value) { return operator=(g() << value); }
};