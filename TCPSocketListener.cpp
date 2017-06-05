#include "TCPSocketListener.h"
#include "types.h"

#include <sstream>

namespace seevider {
	using boost::asio::ip::tcp;

	TCPSocketListener::TCPSocketListener() :
		mHandlerThread(boost::bind(&TCPSocketListener::run, this)) {
	}

	TCPSocketListener::~TCPSocketListener() {
	}

	void TCPSocketListener::run() {
		boost::asio::io_service io_service;
		tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), mPort));

		try {
			while (mOperation) {
				tcp::socket socket_(io_service);
				acceptor_.accept(socket_);
				session(socket_);
			}
		}
		catch (std::exception& e) {
			std::cerr << "Exception occured from TCPSocketListener: " << e.what() << std::endl;
		}
	}

	void TCPSocketListener::session(tcp::socket &sock)
	{
		bool run = true;

		try {
			while (run) {
				char data[1024];

				boost::system::error_code error;
				size_t length = sock.read_some(boost::asio::buffer(data), error);

				if (error == boost::asio::error::eof) {
					break; // Connection closed cleanly by peer.
				}
				else if (error) {
					throw boost::system::system_error(error); // Some other error.
				}

				// parse incomining message
				boost::property_tree::ptree ptree = parseJSON(data);

				int requestCode = ptree.get<int>(JSON_KEY_REQUEST);

				switch (requestCode) {
				case JSON_REQUEST_STREAMING:
					break;

				case JSON_REQUEST_DISCONNECT:
					run = false;
					break;

				default:
					break;
				}

				// send an acknowledgement
				boost::asio::write(sock, boost::asio::buffer(data, length));
			}
		}
		catch (std::exception& e) {
			std::cerr << "Exception in TCPSocketListener::session(): " << e.what() << std::endl;
		}
	}

	boost::property_tree::ptree TCPSocketListener::parseJSON(char *buffer) {
		boost::property_tree::ptree ptree;
		std::stringstream ss;

		ss << buffer;

		boost::property_tree::read_json(buffer, ptree);

		return ptree;
	}
};
