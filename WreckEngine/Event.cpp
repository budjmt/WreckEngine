#include "Event.h"

#include <string>

using namespace Event;

unique_name<Message> Message::names;
unique_name<Trigger> Trigger::names;
unique_name<Handler> Handler::names;

std::unordered_map<uint32_t, Handler*> Dispatcher::handlers;
std::unordered_map<uint32_t, std::vector<Handler*>> Dispatcher::handlerTypes;
Trigger Dispatcher::central_trigger = make_trigger<Dispatcher>(0);

void Dispatcher::sendToHandler(const uint32_t handler_id, Message e) {
	handlers.at(handler_id)->process(e);
}

void Dispatcher::sendToType(const uint32_t type_id, Message e) {
    if (auto typeHandler = handlerTypes.find(type_id); typeHandler != end(handlerTypes)) {
        for (const auto handler : typeHandler->second) {
            handler->process(e);
        }
    }
    for (const auto child_type : unique_type::get_data(type_id).child_ids)
        sendToType(child_type, std::move(e));
}