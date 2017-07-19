#include "SecureClient.h"

#include <iostream>
#include <sstream>

#include <boost/bind.hpp>

#include <glog/logging.h>

SecureClient::SecureClient(boost::asio::io_service& io_service,
	boost::asio::ssl::context& context,
	boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	std::string message)
	: socket_(io_service, context), request_(message)
{
	socket_.set_verify_mode(boost::asio::ssl::verify_none);
	socket_.set_verify_callback(
		boost::bind(&SecureClient::verify_certificate, this, _1, _2));

	boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
		boost::bind(&SecureClient::handle_connect, this,
		boost::asio::placeholders::error));
}

bool SecureClient::verify_certificate(bool preverified,
	boost::asio::ssl::verify_context& ctx)
{
	// The verify callback can be used to check whether the certificate that is
	// being presented is valid for the peer. For example, RFC 2818 describes
	// the steps involved in doing this for HTTPS. Consult the OpenSSL
	// documentation for more details. Note that the callback is called once
	// for each certificate in the certificate chain, starting from the root
	// certificate authority.

	// In this example we will simply print the certificate's subject name.
	char subject_name[256];
	X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
	X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
	LOG(INFO) << "Verifying " << subject_name;

	return preverified;
}

void SecureClient::handle_connect(const boost::system::error_code& error) {
	if (!error)
	{
		LOG(INFO) << "Connection OK!";
		socket_.async_handshake(boost::asio::ssl::stream_base::client,
			boost::bind(&SecureClient::handle_handshake, this,
			boost::asio::placeholders::error));
	}
	else
	{
		LOG(ERROR) << "Connect failed: " << error.message();
	}
}

void SecureClient::handle_handshake(const boost::system::error_code& error) {
	if (!error)
	{
		LOG(INFO) << "Sending request";
		boost::asio::async_write(socket_,
			boost::asio::buffer(request_),
			boost::bind(&SecureClient::handle_write, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		LOG(ERROR) << "Handshake failed: " << error.message();
	}
}

void SecureClient::handle_write(const boost::system::error_code& error,
	size_t bytes_transferred)
{
	if (!error)
	{
		LOG(INFO) << "Sending request OK!";
		boost::asio::async_read_until(socket_,
			reply_, "\r\n",
			boost::bind(&SecureClient::handle_read, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		LOG(ERROR) << "Write failed: " << error.message();
	}
}

void SecureClient::handle_read(const boost::system::error_code& error,
	size_t bytes_transferred)
{
	if (!error)
	{
		std::cout << "Reply:\n" << &reply_ << "\n";
	}
	else
	{
		LOG(ERROR) << "Read failed: " << error.message();
	}
}
