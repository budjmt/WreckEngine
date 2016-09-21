#pragma once

#define DECL_FLD(type, name) type _ ## name

#define DEF_PROP_G(clss, type, access, name)  property<clss, type, access> name = property<clss, type, access>(this, &clss ## ::get_ ## name);
#define DEF_PROP_S(clss, type, access, name)  property<clss, type, access> name = property<clss, type, access>(this, &clss ## ::set_ ## name);
#define DEF_PROP_GS(clss, type, access, name) property<clss, type, access> name = property<clss, type, access>(this, &clss ## ::get_ ## name, &clss ## ::set_ ## name);

#define DEF_GET(type, name, body) type get_ ## name() body;
#define DEF_GET_S(t_get, name) DEF_GET_S_C(t_get, name, { return _ ## name; });
#define DEF_GET_S_C(t_get, name, body) t_get name() const body;

#define DEF_SET(type, name, body) const type& set_ ## name(type value) body;
#define DEF_SET_S(t_set, name) DEF_SET_S_C(t_set, name, { _ ## name = value; });
#define DEF_SET_S_C(t_set, name, body) void name(t_set value) body;

#define PROP_G(clss, type, name, get_body) public: DEF_PROP_G(clss, type, GET, name) private: DEF_GET(type, name, get_body) DECL_FLD(type, name)
#define PROP_S(clss, type, name, set_body) public: DEF_PROP_S(clss, type, SET, name) private: DEF_SET(type, name, set_body) DECL_FLD(type, name)
#define PROP_GS(clss, type, name, get_body, set_body) public: DEF_PROP_GS(clss, type, GET_SET, name) private: DEF_GET(type, name, get_body) DEF_SET(type, name, set_body) DECL_FLD(type, name)

#define ACCS_G(type, name)         public: DEF_GET_S(type, name)         private: DECL_FLD(type, name)
#define ACCS_G_C(type, name, body) public: DEF_GET_S_C(type, name, body) private: DECL_FLD(type, name)
#define ACCS_G_T(type, t_get, name)         public: DEF_GET_S(t_get, name)         private: DECL_FLD(type, name)
#define ACCS_G_T_C(type, t_get, name, body) public: DEF_GET_S_C(t_get, name, body) private: DECL_FLD(type, name)

#define ACCS_S(type, name)         public: DEF_SET_S(type, name)         private: DECL_FLD(type, name)
#define ACCS_S_C(type, name, body) public: DEF_SET_S_C(type, name, body) private: DECL_FLD(type, name)
#define ACCS_S_T(type, t_set, name)         public: DEF_SET_S(t_set, name)         private: DECL_FLD(type, name)
#define ACCS_S_T_C(type, t_set, name, body) public: DEF_SET_S_C(t_set, name, body) private: DECL_FLD(type, name)

#define ACCS_GS(type, name)                       public: DEF_GET_S(type, name)             DEF_SET_S(type, name)             private: DECL_FLD(type, name)
#define ACCS_GS_C(type, name, get_body, set_body) public: DEF_GET_S_C(type, name, get_body) DEF_SET_S_C(type, name, set_body) private: DECL_FLD(type, name)
#define ACCS_GS_T(type, t_get, t_set, name)                       public: DEF_GET_S(t_get, name)             DEF_SET_S(t_set, name)             private: DECL_FLD(type, name)
#define ACCS_GS_T_C(type, t_get, t_set, name, get_body, set_body) public: DEF_GET_S_C(t_get, name, get_body) DEF_SET_S_C(t_set, name, set_body) private: DECL_FLD(type, name)

enum property_type { GET = 1, SET = 2, GET_SET = 3 };
template <typename P, typename T, property_type pt> class property { };

template<class P, class T>
struct property<P, T, GET> {
	property<P, T, GET>(P* parent, T(P::*getter)()) : m_parent(parent), get(getter) { }
	inline T operator()() { return (m_parent->*get)(); }
	inline operator T() { return operator()(); }
private:
	P* m_parent = nullptr;
	T(P::*get)() = nullptr;
};

template<class P, class T>
struct property<P, T, SET> {
	property<P, T, SET>(P* parent, const T&(P::*setter)(T)) : m_parent(parent), set(setter) { }
	inline const T& operator=(T value) { return (m_parent->*set)(value); }
private:
	P* m_parent = nullptr;
	const T&(P::*set)(T) = nullptr;
};

template<class P, class T>
struct property<P, T, GET_SET> {
	property<P, T, GET_SET>(P* parent, T(P::*getter)(), const T&(P::*setter)(T)) : m_parent(parent), get(getter), set(setter) { }
	inline T operator()() { return g(); }
	inline operator T() { return g(); }
	inline const T& operator=(T value) { return (m_parent->*set)(value); }
	inline const T& operator+=(T value) { return operator=(g() + value); }
	inline const T& operator-=(T value) { return operator=(g() - value); }
	inline const T& operator*=(T value) { return operator=(g() * value); }
	inline const T& operator/=(T value) { return operator=(g() / value); }
	inline const T& operator&=(T value) { return operator=(g() & value); }
	inline const T& operator|=(T value) { return operator=(g() | value); }
	inline const T& operator^=(T value) { return operator=(g() ^ value); }
	inline const T& operator>>=(T value) { return operator=(g() >> value); }
	inline const T& operator<<=(T value) { return operator=(g() << value); }
private:
	P* m_parent = nullptr;
	T(P::*get)() = nullptr;
	const T&(P::*set)(T) = nullptr;
	inline T g() { return (m_parent->*get)(); }
};