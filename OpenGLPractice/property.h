#pragma once

#define DECL_PROP_D(type, name, def) type _ ## name = def;
#define DECL_PROP(type, name)        type _ ## name;

#define DEF_PROP_G(clss, type, access, name)  property<clss, type, access> name = property<clss, type, access>(this, &clss ## ::get_ ## name);
#define DEF_PROP_S(clss, type, access, name)  property<clss, type, access> name = property<clss, type, access>(this, &clss ## ::set_ ## name);
#define DEF_PROP_GS(clss, type, access, name) property<clss, type, access> name = property<clss, type, access>(this, &clss ## ::get_ ## name, &clss ## ::set_ ## name);

#define DEF_GET(type, name, body) type get_ ## name() body;
#define DEF_GET_S(type, name) DEF_GET_S_C(type, name, { return _ ## name; });
#define DEF_GET_S_C(type, name, body) type name() const body;

#define DEF_SET(type, name, body) const type& set_ ## name(type value) body;
#define DEF_SET_S(type, name) DEF_SET_S_C(type, name, { _ ## name = value; });
#define DEF_SET_S_C(type, name, body) void name(type value) body;

#define PROP_G_D(clss, type, name, get_body, def) public: DEF_PROP_G(clss, type, GET, name) private: DECL_PROP_D(type, name, def) DEF_GET(type, name, get_body)
#define PROP_G(clss, type, name, get_body)        public: DEF_PROP_G(clss, type, GET, name) private: DECL_PROP(type, name)        DEF_GET(type, name, get_body)

#define PROP_S_D(clss, type, name, set_body, def) public: DEF_PROP_S(clss, type, SET, name) private: DECL_PROP_D(type, name, def) DEF_SET(type, name, set_body)
#define PROP_S(clss, type, name, set_body)        public: DEF_PROP_S(clss, type, SET, name) private: DECL_PROP(type, name)        DEF_SET(type, name, set_body)

#define PROP_GS_D(clss, type, name, get_body, set_body, def) public: DEF_PROP_GS(clss, type, GET_SET, name) private: DECL_PROP_D(type, name, def) DEF_GET(type, name, get_body) DEF_SET(type, name, set_body)
#define PROP_GS(clss, type, name, get_body, set_body)        public: DEF_PROP_GS(clss, type, GET_SET, name) private: DECL_PROP(type, name)        DEF_GET(type, name, get_body) DEF_SET(type, name, set_body)

#define PROP_G_D_S(type, name, def) public: DEF_GET_S(type, name) private: DECL_PROP_D(type, name, def) 
#define PROP_G_S(type, name)        public: DEF_GET_S(type, name) private: DECL_PROP(type, name) 

#define PROP_G_D_S_C(type, name, body, def) public: DEF_GET_S_C(type, name, body) private: DECL_PROP_D(type, name, def) 
#define PROP_G_S_C(type, name, body)        public: DEF_GET_S_C(type, name, body) private: DECL_PROP(type, name) 

#define PROP_S_D_S(type, name, def) public: DEF_SET_S(type, name) private: DECL_PROP_D(type, name, def) 
#define PROP_S_S(type, name)        public: DEF_SET_S(type, name) private: DECL_PROP(type, name) 

#define PROP_S_D_S_C(type, name, body, def) public: DEF_SET_S_C(type, name, body) private: DECL_PROP_D(type, name, def) 
#define PROP_S_S_C(type, name, body)        public: DEF_SET_S_C(type, name, body) private: DECL_PROP(type, name) 

#define PROP_GS_D_S(type, name, def) public: DEF_GET_S(type, name) DEF_SET_S(type, name) private: DECL_PROP_D(type, name, def) 
#define PROP_GS_S(type, name)        public: DEF_GET_S(type, name) DEF_SET_S(type, name) private: DECL_PROP(type, name) 

#define PROP_GS_D_S_C(type, name, get_body, set_body, def) public: DEF_GET_S_C(type, name, get_body) DEF_SET_S_C(type, name, set_body) private: DECL_PROP_D(type, name, def) 
#define PROP_GS_S_C(type, name, get_body, set_body)        public: DEF_GET_S_C(type, name, get_body) DEF_SET_S_C(type, name, set_body) private: DECL_PROP(type, name) 

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