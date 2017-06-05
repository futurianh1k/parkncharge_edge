#pragma once

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace seevider {
	class TCPSocketListener {
	public:
		TCPSocketListener();
		~TCPSocketListener();

	private:
		/**
		 * Network handling thread
		 */
		boost::thread mHandlerThread;

		/**
		 * Default port number to receive the connection
		 */
		const int mPort = 1050;

		/**
		 * The maxmimum buffer size of common messages
		 */
		const int mMaxLength = 1024;

		/**
		 * True while the system is running
		 */
		bool mOperation = true;

		/**
		 * Main entry of the handling thread
		 */
		void run();

		/**
		 * Session
		 */
		void session(boost::asio::ip::tcp::socket &sock);

		/**
		 * Parse JSON structure from given byte array
		 */
		boost::property_tree::ptree parseJSON(char *buffer);

		/**
		 * Parse received request
		 */
		int parseRequest(char *buffer);
	};
};
