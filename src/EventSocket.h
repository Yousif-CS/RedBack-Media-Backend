#ifndef EVENTSOCKETH
#define EVENTSOCKETH

#include <iostream>
#include <string>
#include <functional>
#include <mutex>
#include <thread>
#include <future>
#include <exception>
#include <queue>
#include <map>

#include "jsoncpp/json/json.h"

std::mutex eventQueueMutex;

namespace RedBack {

	// An event based socket that allows to create new events and assign callbacks to them
	template<typename T>
	class EventSocket: public std::enable_shared_from_this<EventSocket<T>> {
	public:
		EventSocket(T& t)
		:t_{t}
		{
			t.set_on_receive([this](std::string payload) { event_enqueue(payload); });
		}
		
		//template<typename R>
		//void set_on_disconnect(std::function<R(std::string)> callback)
		//{
		//	disconnect_callback = callback;
		//}

		//template<typename C>
		//void set_event(std::string eventName, std::function<C(std::string)> callback)
		//{
		//	eventCallbacks[eventName] = callback;
		//}
		//
		////The default callback for a message that is not an event
		//template <typename C>
		//void set_default_callback(std::function<C(std::string)> callback)
		//{
		//	default_callback = callback;
		//}

		// returns the object calculated from the callback(event handler) when the event happens 
		template <typename C>
		std::future<C> listen(std::string eventName, std::function<C(std::string)> callback){
			std::future<C> futureObj = std::async(std::launch::async, &EventSocket<T>::wait_then_perform<C>, this, eventName, callback);
			return futureObj;
		}

	private:

		// Keeps listening for an event and when it happens, calls the "callback"
		template<typename C>
		C wait_then_perform(std::string eventName, std::function<C(std::string)> callback) {
			while (true) {
				if (!eventQueue.empty() && eventName == eventQueue.front().first) {
					break;
				}
			}
			std::lock_guard<std::mutex> guard(eventQueueMutex);
			std::string payload = eventQueue.front().second;
			eventQueue.pop();
			return callback(payload);

		}

		// Forwards an event to the queue that callbacks
		//are listening on
		void event_enqueue(std::string payload) {
			Json::Value root;
			Json::Reader reader;
			std::string errors;
			if (!reader.parse(payload.c_str(), root)) {
				throw std::runtime_error("Error: Could not parse payload");
			}
			std::lock_guard<std::mutex> guard(eventQueueMutex);
			eventQueue.push(std::pair<std::string, std::string>(root["eventName"].asString(), root["payload"].asString()));
		}

		//template<typename C>
		//std::map<std::string, std::function<C(std::string)>> eventCallbacks;
		
		//template<typename R>
		//std::function<R(std::string)> default_callback;

		T& t_;

		std::queue<std::pair<std::string, std::string>> eventQueue;
	};
} // RedBack

#endif