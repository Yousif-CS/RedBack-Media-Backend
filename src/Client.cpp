#include "Client.h"
#include "boost/beast/http.hpp"

namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace RedBack {
	namespace Client {
	    
        template<typename C>
        C establish(std::string host, std::string port, std::function<C(std::shared_ptr<WebSocket>)> callback) {


		}
        
        template<typename C>
        C send(WebSocket& ws, std::string payload, std::function<C(WebSocket& ws)> callback) {
            try {
                ws.write(net::buffer(std::string(payload)));
                return callback(ws);

            }
            catch (std::exception& e) {
                std::cerr << "Error when sending: " << e.what() << std::endl;
            }
        }

        std::string receive(WebSocket& ws) {
            try {
                beast::flat_buffer buffer;
                ws.read(buffer);
                return buffers_to_string(buffer.data());

            }
            catch (std::exception& e) {
                std::cerr << "Error when sending: " << e.what() << std::endl;
            }
        }

        void close(WebSocket& ws) {
            ws.close(websocket::close_code::normal);
        }

	} // Client
} // RedBack