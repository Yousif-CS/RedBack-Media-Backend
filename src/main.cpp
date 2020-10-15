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

int main(int argc, char* argv[]) {
	
	if (argc < 3) {
		std::cout << "Usage: ./" << argv[0] << " " << "[host] [port]" << std::endl;
		return EXIT_SUCCESS;
	}
	RedBack::ListeningServer server{ argv[1], static_cast<unsigned short>(std::atoi(argv[2])) };

	std::vector<
			std::shared_ptr<
					PeerConnectionBuilder<
						RedBack::EventSocket<
							RedBack::WebSocket<tcp::socket>>>>> pcs;
	while (true) {
		std::shared_ptr<RedBack::WebSocket<tcp::socket>> ws = server.accept();
		RedBack::EventSocket<RedBack::WebSocket<tcp::socket>> es{ *ws };
		pcs.push_back(std::make_shared<PeerConnectionBuilder<RedBack::EventSocket<RedBack::WebSocket<tcp::socket>>>>(es));
		std::cout << "Peer Connection request!" << std::endl;
	}

	return 0;
}