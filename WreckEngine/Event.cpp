#include "Event.h"

#include <string>

unique_name<Event> Event::names;
unique_name<EventTrigger> EventTrigger::names;
unique_name<EventHandler> EventHandler::names;

std::unordered_map<uint32_t, EventHandler*> EventDispatcher::handlers;
std::unordered_map<uint32_t, std::vector<EventHandler*>> EventDispatcher::handlerTypes;

void EventDispatcher::sendToHandler(const uint32_t handler_id, Event e) {
	handlers.at(handler_id)->process(std::move(e));
}

void EventDispatcher::sendToType(const uint32_t type_id, Event e) {
	for (const auto handler : handlerTypes.at(type_id)) {
		handler->process(std::move(e));
	}
	for (const auto child_type : unique_type::get_data(type_id).child_ids)
		sendToType(child_type, std::move(e));
}