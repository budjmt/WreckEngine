#pragma once

#include <unordered_map>
#include <functional>

#include "smart_ptr.h"
#include "unique_id.h"

#define ADD_EVENT(event_name) auto event_name ## _event = Event::Message::add(#event_name) 

// Issues:
//  - Handlers use std::function
//      - use of std::function creates a bit of performance degradation at the expense of more flexibility
//      - realistically, this should be a plain old function pointer
//  - owner pointers
//		- if the owning object gets copied and the handler/trigger is a member, the copied handler/trigger is ALSO owned by the original object
//		- assignment is already invalid because of the const data members, but copy or move CONSTRUCTION is fine.
//		- we could overload/delete the copy/move constructors every time, but that's bad

namespace Event {

    // data about a event endpoint, (i.e. trigger or handler) including its type and id
    // [type] = unique_type::index<T>::id
    // [id] = e.g. EventTrigger::get("bomb")
    // [originator] would be the "this" pointer of the Event[Thing] object
    struct EndpointData {
        uint32_t id = 0, type = 0;
        void* originator = nullptr;

        template<class Owner> EndpointData(uint32_t _id, Owner* _originator) : EndpointData(_id, UNIQUE_TYPE_ID(Owner), _originator) {}
        EndpointData(uint32_t _id, uint32_t _type, void* _originator) : id(_id), type(_type), originator(_originator) {}
        EndpointData() = default;
    };

    // Messages contain an identifier, and a bunch of related data on the heap
    // It's possible to send an event with NO data at all, or with lots of data; whatever is necessary
    // These are the *real* events
    struct Message {
        // [members] = the number of pieces of data that get sent with the event
        Message(const std::string name, const EndpointData triggerData, const size_t _members = 0) : Message(get(name), triggerData, _members) {}
        Message(const uint32_t _id, const EndpointData triggerData, const size_t _members = 0) : id(_id), trigger(triggerData), data(void_array(_members)) {}

        // the actual id of the event, indicates what data should be extracted
        // e.g. id == Event::get("explode")
        const uint32_t id;

        // data associated with the trigger
        const EndpointData trigger;

        // the data passed with this Event; the trigger populates and the handler interprets
        void_array data;

        UNIQUE_NAMES(private, Message);
    };

    class Handler;
    class Trigger;

    class Dispatcher {
        static std::unordered_map<uint32_t, Handler*> handlers;
        static std::unordered_map<uint32_t, std::vector<Handler*>> handlerTypes;

    public:
        static void sendToHandler(const uint32_t handler_id, Message e);
        // for standard dispatch, nothing special is required, just pass unique_type::index<T>::id
        // for polymorphic dispatch, either add the PARENT_TYPE macro below the child definition or the CHILD_TYPE macro below the parent definition
        // this will enable the association at runtime
        static void sendToType(const uint32_t type_id, Message e);

        // useful if the event isn't triggered by any object in particular, like input events
        static Trigger central_trigger;

        friend class Handler;
    };

    // Sends Events to the Dispatcher; member of class Owner
    class Trigger {
    public:
        const EndpointData info;

        template<class Owner> Trigger(Owner* _this) : Trigger(_this, Random::get()) {}
        template<class Owner> Trigger(Owner* _this, const std::string name) : Trigger(_this, get(name)) {}
        template<class Owner> Trigger(Owner* _this, const uint32_t _id) : info(EndpointData(_id, _this)) { }

        // sends an event to a single handler
        template<typename... Args>
        void sendEvent(const uint32_t handler_id, const uint32_t event_id, Args&&... messageConstructArgs) {
            Message e(event_id, info, sizeof...(Args));
            e.data.construct(std::forward<Args>(messageConstructArgs)...);
            Event::Dispatcher::sendToHandler(handler_id, std::move(e));
        }

        // sends an event to a group of handlers registered with the same type
        template<typename handler_t, typename... Args>
        void sendBulkEvent(const uint32_t event_id, Args&&... messageConstructArgs) {
            Message e(event_id, info, sizeof...(Args));
            e.data.construct(std::forward<Args>(messageConstructArgs)...);
            Event::Dispatcher::sendToType(UNIQUE_TYPE_ID(handler_t), std::move(e));
        }

        UNIQUE_NAMES(private, Trigger);
    };

    // Handles Events received from the Dispatcher; member of class Owner
    class Handler {
    public:
        typedef const Message& param;
        typedef std::function<void(param)> func;

        const EndpointData info;

        template<class Owner> Handler(Owner* _this, const func f) : Handler(_this, Random::get(), f) {}
        template<class Owner> Handler(Owner* _this, const std::string name, const func f) : Handler(_this, get(name), f) {}
        template<class Owner> Handler(Owner* _this, const uint32_t _id, const func f) : info(EndpointData(_id, _this)), handler(f) { register_self(); }

        Handler(const Handler& other) : info(other.info), handler(other.handler) { register_self(); }
        Handler(Handler&& other) : info(other.info), handler(other.handler) { register_self(); }
        ~Handler() { unregister_self(); }

        func handler;
        
        // note that this handler WILL be bound to a specific object, which won't be updated on delete or copy
        // take that into account when assigning an object to handle it
        template<class Owner> 
        static func wrap_member_func(Owner* _this, void (Owner::*member_handler)(param)) { 
            return [_this, member_handler](param e) { 
                (_this->*member_handler)(e); 
            }; 
        }

        // processes an Event received from the dispatcher through the handler function
        inline void process(const Message& e) { handler(e); }


        UNIQUE_NAMES(private, Handler);

    private:

        void register_self() {
            Dispatcher::handlers.insert({ info.id, this });
            Dispatcher::handlerTypes[info.type].push_back(this);
        }

        void unregister_self() {
            Dispatcher::handlers.erase(info.id);
            auto t_handlers = Dispatcher::handlerTypes[info.type];
            t_handlers.erase(std::find(t_handlers.begin(), t_handlers.end(), this));
        }

    };

    template<typename T, typename... Args>
    static Trigger make_trigger(Args&&... args) { return Trigger((T*) nullptr, std::forward<Args>(args)...); }

    template<typename T, typename... Args>
    static Handler make_handler(Args&&... args) { return Handler((T*) nullptr, std::forward<Args>(args)...); }

}