// File:	TCPSocketListener.cpp
// Author:	Seongdo Kim
// Contact:	sdland85@gmail.com
//
// Copyright (c) 2017, Seongdo Kim <sdland85@gmail.com>
// All rights reserved.
// The copyright to the computer program(s) herein is
// the property of Seongdo Kim. The program(s) may be
// used and/or copied only with the written permission
// of Seongdo Kim or in accordance with the terms and
// conditions stipulated in the agreement/contract under
// which the program(s) have been supplied.
//
// Written by Seongdo Kim <sdland85@gmail.com>, June, 2017

#include "TCPSocketListener.h"
#include "types.h"

#include <sstream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <glog/logging.h>

namespace seevider {
	using boost::asio::ip::tcp;

	TCPSocketListener::TCPSocketListener(
		std::shared_ptr<ParkingSpotManager> &parkingSpotManager,
		std::shared_ptr<SerialVideoReader> &serialVideoReader,
		std::shared_ptr<Settings> &settings,
		MutualConditionVariable& mutualCV) :
		mParkingSpotManager(parkingSpotManager), mSerialVideoReader(serialVideoReader),
		mSettings(settings), mMutualConditionVariable(mutualCV) {
		// Start the handling thread
		boost::thread t(boost::bind(&TCPSocketListener::run, this));
		mHandlerThread.swap(t);

		RequestTranslator["disconnect"] = REQ_DISCONNECT;
		RequestTranslator["streaming"] = REQ_STREAMING;
		RequestTranslator["update"] = REQ_UPDATE;
	}

	TCPSocketListener::~TCPSocketListener() {
		if (mHandlerThread.joinable()) {
			// If the thread is still joinable, destroy it.
			destroy();
		}
	}

	void TCPSocketListener::destroy() {
		LOG(INFO) << "Destroying the socket listener";

		mOperation = false;
		mHandlerThread.try_join_for(boost::chrono::seconds(mWaitSeconds));
		if (mHandlerThread.joinable()) {
			// If the thread is still joinable, force to cancel it.
			mHandlerThread.interrupt();
			mHandlerThread.try_join_for(boost::chrono::seconds(mDestroySeconds));
		}

		LOG(INFO) << "The socket listener has destroyed";
	}

	void TCPSocketListener::run() {
		LOG(INFO) << "Starting the socket listener";

		boost::asio::io_service io_service;
		tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), Port));
		io_service.run();

		try {
			while (mOperation) {
				tcp::socket socket_(io_service);
				acceptor_.accept(socket_);
				LOG(INFO) << "Connection established with " << socket_.remote_endpoint().address().to_string();
				session(socket_);
				LOG(INFO) << "Connection destroyed";
			}
		}
		catch (std::exception& e) {
			LOG(ERROR) << e.what();
		}
	}

	void TCPSocketListener::session(tcp::socket &sock)
	{
		bool connected = true;
		boost::system::error_code error;

		try {
			// Send a welcome message
			boost::asio::write(sock, boost::asio::buffer(getJSONSyncMessage()), error);

			if (error) {
				DLOG(ERROR) << error.message();
			}

			while (connected) {
				char data[MaxBufSize];

				std::memset(data, 0, sizeof(char) * 4096);

				size_t length = sock.read_some(boost::asio::buffer(data));

				if (error == boost::asio::error::eof) {
					break; // Connection closed cleanly by peer.
				}
				else if (error) {
					LOG(ERROR) << error.message();
					throw boost::system::system_error(error); // Some other error.
				}
				
				// parse incomining request message
				boost::property_tree::ptree json = parseJSON(data);
				
				int requestCode = parseRequestCode(json);

				switch (requestCode) {
				case REQ_STREAMING:
					//DLOG(INFO) << "REQ_STREAMING";
					sendImageFrame(sock);
					break;

				case REQ_UPDATE:
					DLOG(INFO) << "REQ_UPDATE";
					update(sock, json);
					break;

				case REQ_DISCONNECT:
					// TODO: disconnection message never have arrived.
					DLOG(INFO) << "REQ_DISCONNECT";
					boost::asio::write(sock, boost::asio::buffer("ok"), error);
					sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
					sock.close();
					connected = false;
					break;

				default:
					break;
				}
				
				// send an acknowledgement
				//boost::asio::write(sock, boost::asio::buffer(data, length));
			}
		}
		catch (std::exception& e) {
			if (error == boost::asio::error::eof) {
				// Connection closed cleanly by peer.
				LOG(INFO) << "Connection closed by peer";
			}
			else if (error) {
				LOG(ERROR) << error.message() << ", " << error.value();
				LOG(ERROR) << e.what();
			}
			else {
				LOG(INFO) << "Connection closed";
			}
		}
	}

	int TCPSocketListener::parseRequestCode(boost::property_tree::ptree &node) {
		std::string requestString = node.get<std::string>(JSON_KEY_REQUEST);
		const auto& elem = RequestTranslator.find(requestString);

		if (elem == RequestTranslator.cend()) {
			return -1;
		}

		return elem->second;
	}

	boost::property_tree::ptree TCPSocketListener::parseJSON(char *buffer) {
		boost::property_tree::ptree ptree;
		std::stringstream ss;

		ss << buffer;

		boost::property_tree::read_json(ss, ptree);

		return ptree;
	}

	boost::property_tree::ptree TCPSocketListener::parseJSON(boost::asio::streambuf& buffer) {
		boost::property_tree::ptree ptree;
		std::istream is(&buffer);

		boost::property_tree::read_json(is, ptree);

		return ptree;
	}

	void TCPSocketListener::sendImageFrame(boost::asio::ip::tcp::socket &sock) {
		std::vector<uchar> imageBuffer;
		std::vector<int> params;
		cv::Mat frame = mSerialVideoReader->read();
		boost::system::error_code error;
		std::stringstream ss;
		char data[64];

		// Initialize
		std::memset(data, 0, sizeof(char) * 64);

		// TODO: handler error if the frame is empty

		// encode image
		params.push_back(cv::IMWRITE_JPEG_QUALITY);
		params.push_back(80);
		cv::imencode(".jpg", frame, imageBuffer, params);

		// Send the image data size to the client
		ss << "{ \"bytes\" : " << imageBuffer.size() << " } ";
		/*ss << "{ \"bytes\" : \"" << imageBuffer.size() << "\" , \"update\" : [ ";
		for (const auto& elem : *mParkingSpotManager) {
			if (elem.first != mParkingSpotManager->begin()->first) {
				ss << ", ";
			}
			
			if (elem.second->isOccupied()) {
				if (elem.second->isOverstayed()) {
					ss << "{ \"id\" : " << elem.second->ID << " , \"code\" : 2 }";	// overstayed
				}
				else {
					ss << "{ \"id\" : " << elem.second->ID << " , \"code\" : 1 }";	// occupied
				}
			}
			else {
				ss << "{ \"id\" : " << elem.second->ID << " , \"code\" : 0 }";	// empty
			}

		}
		ss << " ] }";*/
		boost::asio::write(sock, boost::asio::buffer(ss.str()), error);
		if (error) {
			LOG(ERROR) << error.message();
		}

		// Receive any message from the connected client
		size_t length = sock.read_some(boost::asio::buffer(data), error);
		if (error) {
			LOG(ERROR) << error.message();
		}

		// Send the image data to the client
		boost::asio::write(sock, boost::asio::buffer(imageBuffer), error);
		if (error) {
			LOG(ERROR) << error.message();
		}
	}

	void TCPSocketListener::update(boost::asio::ip::tcp::socket &sock, boost::property_tree::ptree& message) {
		boost::system::error_code error;
		int bytes_reserved = message.get<int>("length");
		int bytes_received;
		boost::asio::streambuf read_buffer;
		std::string input;

		// Suspend the main thread for update
		suspendMainThread();

		// Send the image data size to the client
		boost::asio::write(sock, boost::asio::buffer("{ \"response\" : \"ok\" }"), error);

		bytes_received = boost::asio::read(sock, read_buffer, boost::asio::transfer_exactly(bytes_reserved));

		message = parseJSON(read_buffer);

		mParkingSpotManager->updateParkingSpots(message.get_child("ROI"));
		
		// Resume the main thread for update
		resumeMainThread();
	}

	void TCPSocketListener::suspendMainThread() {
		DLOG(INFO) << "Turn on the management mode";
		boost::mutex::scoped_lock lock(mMutualConditionVariable.MutexEntrace);
		mMutualConditionVariable.ManagementMode = true;
		mMutualConditionVariable.SenderCV.wait(lock);
	}

	void TCPSocketListener::resumeMainThread() {
		DLOG(INFO) << "Turn off the management mode";
		mMutualConditionVariable.ManagementMode = false;
		mMutualConditionVariable.ReceiverCV.notify_all();	// resume the main thread
	}

	std::string TCPSocketListener::getJSONSyncMessage() {
		boost::property_tree::ptree message;
		std::stringstream ss;

		// Welcome message
		message.put<std::string>("welcome", "Hello message from the sensor.");

		// Garage category
		message.put<std::string>("category", "sample_category");

		// Sensor ID
		message.put<std::string>("id", mSettings->SensorID);

		// Sensor name
		message.put<std::string>("name", mSettings->SensorName);

		// sensitivity -- dummy
		message.put<int>("sensitivity", 3);

		// enter count -- dummy
		message.put<int>("enterCount", 6);

		// exit count -- dummy
		message.put<int>("exitCount", 3);

		// ROI
		message.add_child("ROI", mParkingSpotManager->toPTree());

		write_json(ss, message);

		return ss.str();
	}

	//--------------------------------
	// Static variable initializations
	//--------------------------------
	const std::string TCPSocketListener::JSON_KEY_REQUEST = "request";

	std::unordered_map<std::string, int> TCPSocketListener::RequestTranslator;
};
