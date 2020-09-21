#include <boost/asio.hpp>

int main() {
	boost::asio::io_service io_service_object;
	boost::asio::ip::tcp::socket socket_object(io_service_object);

	socket_object.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("localhost"), 6666));

	boost::asio::write(socket_object, boost::asio::buffer("Hey Man!"));
}