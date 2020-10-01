#include <functional>

#include "CustomWebSocket.h"
#include "ListeningServer.h"
#include "EventSocket.h"
#include "PeerConnectionBuilder.h"

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
		// PeerConnectionBuilder<RedBack::EventSocket<RedBack::WebSocket<tcp::socket>>> pcb{es};
		// pcb.get_peer_connection();
		// std::cout << "Peer Connection established!" << std::endl;
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