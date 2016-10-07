#pragma once

#include <unordered_map>
#include <functional>

#include "smart_ptr.h"
#include "unique_id.h"

// data about a event endpoint, (i.e. trigger or handler) including its type and id
// [type] = unique_type<T>::id
// [id] = e.g. EventTrigger::get("bomb")
// [originator] would be the "this" pointer of the Event[Thing] object
struct EventEndpointData { 
	uint32_t type, id; 
	void* originator; 

	EventEndpointData(uint32_t _type, uint32_t _id, void* _originator) : type(_type), id(_id), originator(_originator) {}
	template<class Owner> EventEndpointData(uint32_t _id, Owner* _originator) : type(unique_type<Owner>::id), id(_id), originator(_originator) {}
};

// this is bad practice, but the original is a pain to type
#define EvED EventEndpointData

// Events contain an identifier, and a bunch of related data on the heap
// It's possible to send an event with NO data at all, or with lots of data; whatever is necessary
struct Event {
	// [members] = the number of pieces of data that get sent with the event
	Event(const std::string name, const EvED triggerData, const size_t members = 0) : Event(get(name), triggerData) { }
	Event(const uint32_t _id, const EvED triggerData, const size_t _members = 0)
		: id(_id), trigger(triggerData), members(_members), data(make_unique<shared<void>[]>(_members)) { }

	// the actual id of the event, indicates what data should be extracted
	// e.g. id == Event::get("explode")
	const uint32_t id;

	// data associated with the trigger
	const EvED trigger;

	// this is reserved on the heap and can hold anything; the trigger populates and the handler interprets
	// intended to function as a basic, immutable size (hence why it's not std::vector) stack
	unique<shared<void>[]> data; size_t size = 0;
	const size_t members;

	// peeks at/gets the data [index] elements into the data stack and puts it on the stack as a variable of type T
	template<typename T> inline T peek(const size_t index) { return *reinterpret_cast<T*>(data[index].get()); }

	// intended for simple stack-allocated types; pushes an element of type T from the stack onto the heap into the data stack
	template<typename T> inline void push(T& d) { data[size++] = shared<T>(new T(d)); }

	// populates an Event object with the variables passed as parameters; types are deduced
	template<typename T, typename... Args>
	void construct(T t, Args&&... args) { push(t); construct(std::forward<Args>(args)...); }
	void construct() {}

	// extracts the contents of the Event data in the form of a tuple on the stack, according to the explicit types passed
	template<typename T>
	std::tuple<T> extractData() { return getDataAsTuple<T>(members - 1); }
	template<typename T1, typename T2, typename... Args>
	std::tuple<T1, T2, Args...> extractData() { return std::tuple_cat(getDataAsTuple<T1>(members - sizeof...(Args)-2), extractData<T2, Args...>()); }

	UNIQUE_NAMES(private, Event);

private:
	template<typename T>
	inline std::tuple<T> getDataAsTuple(const size_t index) {
		return std::tuple<T>(peek(index));
	}
};

class EventHandler;

class EventDispatcher {
	static std::unordered_map<uint32_t, EventHandler*> handlers;
	static std::unordered_map<uint32_t, std::vector<EventHandler*>> handlerTypes; // TODO make this work with polymorphic types?

public:
	static void sendToHandler(const uint32_t handler_id, Event e);
	static void sendToType(const uint32_t type_id, Event e);

	friend class EventHandler;
};

// Sends Events to the EventDispatcher; member of class Owner
class EventTrigger {
public:
	const EvED info;

	template<class Owner> EventTrigger(Owner* _this) : EventTrigger(_this, rand()) {}
	template<class Owner> EventTrigger(Owner* _this, const std::string name) : EventTrigger(_this, get(name)) {}
	template<class Owner> EventTrigger(Owner* _this, const uint32_t _id) : info(EvED(_id, _this)) {}

	// sends an event to a single handler
	template<typename... Args>
	void sendEvent(const uint32_t handler_id, const uint32_t event_id, Args&&... args) {
		Event e(event_id, info, sizeof...(Args));
		e.construct(std::forward<Args>(args)...);
		EventDispatcher::sendToHandler(handler_id, std::move(e));
	}

	// sends an event to a group of handlers registered with the same type
	template<typename T, typename... Args>
	void sendBulkEvent(const uint32_t event_id, Args&&... args) {
		Event e(event_id, info, sizeof...(Args));
		e.construct(std::forward<Args>(args)...);
		EventDispatcher::sendToType(unique_type<T>::id, std::move(e));
	}

	UNIQUE_NAMES(private, EventTrigger);
};

// Handles Events received from the EventDispatcher; member of class Owner
class EventHandler {
public:
	const EvED info;

	template<class Owner> EventHandler(Owner* _this, const std::function<void(Event)> f) : EventHandler(_this, rand(), f) {}
	template<class Owner> EventHandler(Owner* _this, const std::string name, const std::function<void(Event)> f) : EventHandler(_this, get(name), f) {	}
	template<class Owner> EventHandler(Owner* _this, const uint32_t _id, const std::function<void(Event)> f) : info(EvED(_id, _this)), handler(f) {
		EventDispatcher::handlers.insert({ info.id, this });
		EventDispatcher::handlerTypes[info.type].push_back(this);
	}

	// processes an Event received from the dispatcher through the handler function
	inline void process(Event e) { handler(std::move(e)); }

	std::function<void(Event)> handler;

	UNIQUE_NAMES(private, EventHandler);
};

#undef EvED