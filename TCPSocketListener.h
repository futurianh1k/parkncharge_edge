// File:	TCPSocketListener.h
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

#pragma once

#include "INetworkHandler.h"
#include "ParkingSpotManager.h"
#include "Settings.h"
#include "SerialVideoReader.h"

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>

namespace seevider {
	class TCPSocketListener : public INetworkHandler {
	private:

		/**
		 * Mutual condition variable to suspend the processing of the main thread.
		 * Use this variable to update critical data that may causes the error if
		 * update without suspending the main thread's processing.
		 */
		MutualConditionVariable &mMutualConditionVariable;

		/**
		 * Manager parking spots
		 */
		std::shared_ptr<ParkingSpotManager> mParkingSpotManager;

		/**
		 * Serial video reader to acquire latest frames to be sent through the socket
		 */
		std::shared_ptr<SerialVideoReader> mSerialVideoReader;

		/**
		 * Sensor information
		 */
		std::shared_ptr<Settings> mSettings = nullptr;

	public:
		/**
		 * Basic constructor
		 */
		TCPSocketListener(
			std::shared_ptr<ParkingSpotManager> &parkingSpotManager,
			std::shared_ptr<SerialVideoReader> &serialVideoReader,
			std::shared_ptr<Settings> &settings,
			MutualConditionVariable& mutualCV);

		/**
		 * Basic destructor
		 */
		~TCPSocketListener();

		/**
		 * Destroy the network thread
		 */
		void destroy() override;

	private:

		/**
		 * Main entry of the handling thread
		 */
		void run() override;

		/**
		 * Session
		 */
		void session(boost::asio::ip::tcp::socket &sock);

		/**
		 * Parse request code from given property tree. If request code does not exist in given tree,
		 * it will return negative number.
		 */
		int parseRequestCode(boost::property_tree::ptree &node);

		/**
		 * Parse JSON structure from given byte array
		 */
		boost::property_tree::ptree parseJSON(char *buffer);

		/**
		 * Parse JSON structure from given stream buffer
		 */
		boost::property_tree::ptree parseJSON(boost::asio::streambuf& buffer);

		/**
		 * Send a message with acknowledgement in following order:
		 * 1) Send a JSON object containing message size to the connected client.
		 * 2) Receive an acknowledgement
		 * 3) Send the given message
		 */
		void send(boost::asio::ip::tcp::socket &sock, const std::string message);

		/**
		 * Send the latest image frame in following order:
		 * 1) Send a JSON object containing image size to the connected client.
		 * 2) Receive any message from the connected client.
		 * 3) Send actual byte array of the image frame.
		 */
		void sendImageFrame(boost::asio::ip::tcp::socket &sock, bool drawStatus);

		/**
		 * Suspend main thread
		 */
		void suspendMainThread();

		/**
		 * Release main thread. Must be called after suspendMainThread() has called.
		 */
		void resumeMainThread();

		/**
		 * Handle update message
		 */
		void update(boost::asio::ip::tcp::socket &sock, boost::property_tree::ptree &message);

		/**
		 * Construct JSON welcome message
		 */
		std::string getJSONSyncMessage();

	public:
		/**
		 * Disconnect request
		 */
		static const int REQ_DISCONNECT = 0;

		/**
		 * Update parking spot information
		 */
		static const int REQ_UPDATE = 1;

		/**
		 * Manager requesting the latest image frame
		 */
		static const int REQ_STREAMING = 2;
		
		/**
		 * Turn on the sensor
		 */
		static const int REQ_SENSORON = 3;

		/**
		 * Turn off the sensor
		 */
		static const int REQ_SENSOROFF = 4;


	private:

		/**
		* Default port number to receive the connection
		*/
		static const int Port = 1050;

		/**
		 * The maxmimum buffer size of common messages
		 */
		static const int MaxBufSize = 4096;

		/**
		 * JSON request key
		 */
		static const std::string JSON_KEY_REQUEST;

		/**
		 * Request translator
		 */
		static std::unordered_map<std::string, int> RequestTranslator;
	};
};
