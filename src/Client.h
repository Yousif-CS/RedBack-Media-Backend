#ifndef CLIENTH
#define CLIENTH

#include <iostream>
#include <string>
#include <functional>

#include "boost/beast/websocket.hpp"
#include "boost/asio.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/beast/core/buffers_to_string.hpp"
#include "boost/beast/http.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace net = boost::asio;            // from <boost/asio.hpp>
using namespace boost::beast;
using namespace boost::beast::websocket;
using tcp = boost::asio::ip::tcp;
namespace http = beast::http;           // from <boost/beast/http.hpp>

namespace RedBack {
	namespace Client {
		#define WebSocketClient websocket::stream<tcp::socket>
		
		// Creates a websocket object that is ready to send and receive information 

        template<typename C>
        C establish(std::string host, std::string port, std::function<C(WebSocketClient&)> callback) {
            try
            {
                // The io_context is required for all I/O
                net::io_context ioc;

                // These objects perform our I/O
                tcp::resolver resolver{ ioc };
                websocket::stream<tcp::socket> ws{ ioc };

                // Look up the domain name
                auto const results = resolver.resolve(host, port);

                // Make the connection on the IP address we get from a lookup
                auto ep = net::connect(ws.next_layer(), results);

                // Update the host_ string. This will provide the value of the
                // Host HTTP header during the WebSocket handshake.
                // See https://tools.ietf.org/html/rfc7230#section-5.4
                host += ':' + std::to_string(ep.port());

                // Set a decorator to change the User-Agent of the handshake
                ws.set_option(websocket::stream_base::decorator(
                    [](websocket::request_type& req)
                    {
                        req.set(http::field::user_agent,
                            std::string(BOOST_BEAST_VERSION_STRING) +
                            "RedBack-Media-Client 1.0");
                    }));

                // Perform the websocket handshake
                ws.handshake(host, "/");
                return callback(ws);
            }
            catch (std::exception const& e)
            {
                std::cerr << "Error at establish: " << e.what() << std::endl;
            }
		}

		// Send information
        template<typename C>
        C send(WebSocketClient& ws, std::string payload, std::function<C(WebSocketClient&)> callback) {
            try {
                ws.write(net::buffer(std::string(payload)));
                return callback(ws);
            }
            catch (std::exception& e) {
                std::cerr << "Error when sending: " << e.what() << std::endl;
            }
        }

        std::string receive(std::shared_ptr<WebSocketClient> ws) {
            try {
                beast::flat_buffer buffer;
                ws->read(buffer);
                return buffers_to_string(buffer.data());

            }
            catch (std::exception& e) {
                std::cerr << "Error when receiving: " << e.what() << std::endl;
            }
        }

        void close(std::shared_ptr<WebSocketClient> ws) {
            ws->close(websocket::close_code::normal);
        }

	} // Client
} // RedBack

#endif