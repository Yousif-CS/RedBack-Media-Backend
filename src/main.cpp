#include <functional>

#include "CustomWebSocket.h"
#include "ListeningServer.h"
#include "EventSocket.h"
#include "api/create_peerconnection_factory.h"

#include "api/peer_connection_interface.h"

//#include "jsoncpp/json/json.h"
//#include "jsoncpp/json/value.h"

//#include "Client.h"

#include "boost/asio/ip/tcp.hpp"
using tcp = boost::asio::ip::tcp;

//void emitEvent(WebSocketClient& ws,
//	std::string eventName, std::string payload, std::function<void(WebSocketClient&)> callback) {
//	Json::Value root;
//	root["eventName"] = eventName;
//	root["payload"] = payload;
//	Json::FastWriter writer;
//	RedBack::Client::send<void>(ws, writer.write(root), callback);
//	
//}
int main(int argc, char* argv[]) {
	
	if (argc < 3) {
		std::cout << "Usage: ./" << argv[0] << " " << "[host] [port]" << std::endl;
		return EXIT_SUCCESS;
	}
	RedBack::ListeningServer server{ argv[1], static_cast<unsigned short>(std::atoi(argv[2])) };

	while (true) {
		std::shared_ptr<RedBack::WebSocket<tcp::socket>> ws = server.accept();
		RedBack::EventSocket<RedBack::WebSocket<tcp::socket>> es{ *ws };


		auto futureObject1 = es.listen<std::string>("Hello", [](std::string payload){
			return payload;
		});

		auto futureObject2 = es.listen<std::string>("World", [](std::string payload) {
			return payload;
		});

		std::cout << "World Event: " << futureObject2.get() << std::endl;
		std::cout << "Hello Event: " << futureObject1.get() << std::endl;
	}
	//RedBack::Client::establish<void>("localhost", "6969", [argv](WebSocketClient& ws) {
	//	std::string eventName;
	//	std::string payload;
	//	while (true) {
	//		std::cin >> eventName >> payload;
	//		emitEvent(ws, eventName, payload, [](WebSocketClient& ws) {});
	//	}
	//});

	return 0;
}