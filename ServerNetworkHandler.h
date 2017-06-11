#pragma once

#include <iostream>
#include <unordered_map>

#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>

#include "types.h"
#include "INetworkHandler.h"
#include "MessageQueue.h"
#include "SensorInfo.h"

namespace seevider {
	class ServerNetworkHandler : public INetworkHandler {
    public:
        /**
        * Construct the network handler with a message queue
        */
		ServerNetworkHandler(std::shared_ptr<MessageQueue> &messageQueue,
			const std::shared_ptr<SensorInfo> &sensorInfo, std::string serverDataFilename);

        /**
        * Destruct the network handler
        */
		~ServerNetworkHandler();

		/**
		 * Destroy the network thread
		 */
		void destroy() override;

    private:
		//---------------------------
		// Private variables
		//---------------------------

        /**
         * The pointer of message queue
         */
		std::shared_ptr<MessageQueue> mMessageQueue = nullptr;

		/**
		 * Sensor information to be sent along with a message
		 */
		const std::shared_ptr<SensorInfo> mSensorInfo = nullptr;
		
		/**
		 * Server address
		 */
		std::string mServerAddr;

		/**
		 * Target destinations
		 */
		std::vector<std::string> mSubAddr;

		/**
		 * Http message uploading methods
		 */
		std::vector<std::string> mMethod;

		/**
		 * Filename to save and load setting data
		 */
		std::string mSettingsFilename;

		/**
		 * Destinations of the server side for each message
		 */
		std::unordered_map<int, seevider::ServerDestinations_t> mServerDestinations;

		//---------------------------
		// Private functions
		//---------------------------

        /**
         * Main entry of the handling thread
         */
		void run() override;

        /**
         * Retrive data from the network message queue, then upload it to the server
         */
		bool upload(const std::unique_ptr<IMessageData> &data) const;

		/**
		 * Send http post message.
		 */
		bool sendHTTP(const std::string method, const std::string jsonstring, const std::string targetAddr) const;

		/**
		 * Load setting data
		 */
		bool loadSettings(const std::string filename);

		/**
		 * Save setting data
		 */
		bool writeSettings();
    };
}