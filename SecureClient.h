#pragma once

#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

class SecureClient
{
public:
	SecureClient(boost::asio::io_service& io_service,
		boost::asio::ssl::context& context,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		std::string message);

	bool verify_certificate(bool preverified,
		boost::asio::ssl::verify_context& ctx);

	void handle_connect(const boost::system::error_code& error);

	void handle_handshake(const boost::system::error_code& error);

	void handle_write(const boost::system::error_code& error,
		size_t bytes_transferred);

	void handle_read(const boost::system::error_code& error,
		size_t bytes_transferred);

private:
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
	std::string request_;
	boost::asio::streambuf reply_;
};
