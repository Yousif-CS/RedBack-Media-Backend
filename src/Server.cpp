#pragma once
#include <iostream>
#include <cstdlib>
#include <functional>
#include <string>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace ssl = boost::asio::ssl;
using namespace boost::beast;
using namespace boost::beast::websocket;
using tcp = boost::asio::ip::tcp;

void do_session(tcp::socket socket) {
	try {
		//construct the websocket
		websocket::stream<tcp::socket> ws{ std::move(socket) };
		
		ws.set_option(websocket::stream_base::decorator(
			[](websocket::response_type& res) {
				res.set(http::field::server,
					std::string(BOOST_BEAST_VERSION_STRING) +
					" RedBack-Media-Backend");
			}
		));

		ws.accept();
		for (;;) {
			//buffer
			beast::flat_buffer buffer;
			ws.text(ws.got_text());
			ws.read(buffer);
			ws.write(buffer.data());
		}
	}
	catch (beast::system_error const& e) {
		//not a closed connection...
		if (e.code() != websocket::error::closed) {
			std::cerr << "Error: " << e.code().message() << std::endl;
		}
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

int main(int argc, char* argv[]) {

	try {
		if (argc != 3) {
			std::cerr << 
				"Usage: " + static_cast<std::string>(argv[0]) + "[address] [port]" << std::endl;
		}

		auto address = net::ip::make_address(argv[1]);
		auto port = static_cast<unsigned short>(std::atoi(argv[2]));

		net::io_context ioc;

		tcp::acceptor acceptor{ ioc, {address, port} };

		for (;;) {

			//this will receive a new connection
			tcp::socket socket{ ioc };

			//block until we get a connection
			acceptor.accept(socket);

			//launch the session
			std::thread(
				&do_session,
				std::move(socket)).detach();

		}

	}
	catch (const std::exception& e) {
		std::cerr << "Error at main: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
