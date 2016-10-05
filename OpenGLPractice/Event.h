#pragma once

#include <vector>
#include <unordered_map>
#include <functional>

#include "smart_ptr.h"
#include "unique_id.h"

struct TriggerData { const uint32_t type, id; void* originator; };

// Events contain an identifier, and a bunch of related data on the heap
// It's possible to send an event with NO data at all, or with lots of data; whatever is necessary
struct Event {
	// [members] = the number of pieces of data that get sent with the event
	Event(const std::string name, const TriggerData triggerData, const size_t members = 0)
		: Event(get(name), triggerData) { }

	Event(const uint32_t _id, const TriggerData triggerData, const size_t _members = 0)
		: id(_id), trigger(triggerData), members(_members), data(make_unique<shared<void>[]>(_members)) { }

	// the actual id of the event, indicates what data should be extracted
	const uint32_t id;

	// data about the trigger, including its type and id
	const TriggerData trigger;

	// this can hold anything reserved on the heap; the trigger populates and the handler interprets
	unique<shared<void>[]> data; size_t size = 0;
	const size_t members;

	template<typename T> T getIndexAs(const size_t index) { return *reinterpret_cast<T*>(data[index].get()); }

	// intended for simple stack-allocated types
	template<typename T> void push(T& d) { data[size++] = shared<T>(new T(d)); }

	template<typename T, typename... Args>
	void construct(T t, Args&&... args) { push(t); construct(std::forward<Args>(args)...); }
	void construct() {}

	// extracts the contents of the data structure in the form of a tuple, assuming the types are passed properly
	template<typename T>
	std::tuple<T> extractData() { return getDataAsTuple<T>(members - 1); }
	template<typename T1, typename T2, typename... Args>
	std::tuple<T1, T2, Args...> extractData() { return std::tuple_cat(getDataAsTuple<T1>(members - sizeof...(Args)-2), extractData<T2, Args...>()); }

	UNIQUE_NAMES(private, Event);

private:
	template<typename T>
	std::tuple<T> getDataAsTuple(const size_t index) {
		return std::tuple<T>(*reinterpret_cast<T*>(data[index].get()));
	}
};

struct EventData {

};

class EventHandler;

class EventDispatcher {
	static std::unordered_map<uint32_t, EventHandler*> handlers;
	static std::unordered_map<uint32_t, std::vector<EventHandler*>> handlerTypes;

public:
	static void sendToHandler(const uint32_t handler_id, Event e);
	static void sendToType(const uint32_t type_id, Event e);

	friend class EventHandler;
};

class EventTriggers { public: UNIQUE_NAMES(private, EventTriggers); };

// construct with id = this
template<class O>// type of owner
class EventTrigger : public EventTriggers {
public:
	const uint32_t id = rand();
	O* owner;

	EventTrigger(O* _this) : owner(_this) {}
	EventTrigger(O* _this, const std::string name) : owner(_this), id(get(name)) {}
	EventTrigger(O* _this, const uint32_t _id) : owner(_this), id(_id) {}

	template<typename... Args>
	void sendEvent(const uint32_t handler_id, const uint32_t event_id, Args&&... args) {
		Event e(event_id, { unique_type<O>::id, id, owner }, sizeof...(Args));
		e.construct(std::forward<Args>(args)...);
		EventDispatcher::sendToHandler(handler_id, std::move(e));
	}

	// sends an event to a group of handlers registered with the same type
	template<typename T, typename... Args>
	void sendBulkEvent(const uint32_t event_id, Args&&... args) {
		Event e(event_id, { unique_type<O>::id, id, owner }, sizeof...(Args));
		e.construct(std::forward<Args>(args)...);
		EventDispatcher::sendToType(unique_type<T>::id, std::move(e));
	}
};

class EventHandler {
public:
	const uint32_t id, type_id;

	EventHandler(const uint32_t _type_id, const uint32_t _id, const std::function<void(Event)> f) : id(_id), type_id(_type_id), handler(f) {
		EventDispatcher::handlers.insert({ id, this });
		EventDispatcher::handlerTypes[type_id].push_back(this);
	}
	EventHandler(const uint32_t _type_id, const std::string name, const std::function<void(Event)> f) : EventHandler(_type_id, get(name), f) {	}
	EventHandler(const uint32_t _type_id, const std::function<void(Event)> f) : EventHandler(_type_id, rand(), f) {}

	void process(Event e) { handler(std::move(e)); }

	UNIQUE_NAMES(private, EventHandler);

private:
	const std::function<void(Event)> handler;
};