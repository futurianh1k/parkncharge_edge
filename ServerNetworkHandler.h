#pragma once

#include <iostream>
#include <unordered_map>

#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>

#include "MessageQueue.h"
#include "Settings.h"

namespace seevider {
    class ServerNetworkHandler {
    public:
        /**
        * Construct the network handler with a message queue
        */
		ServerNetworkHandler(std::shared_ptr<MessageQueue> &messageQueue, const std::shared_ptr<Settings> &settings);

        /**
        * Destruct the network handler
        */
		~ServerNetworkHandler();

		/**
		 * Destroy the network thread
		 */
		void destroy();

    private:
		//---------------------------
		// Private variables
		//---------------------------

        /**
         * Network handling thread
         */
        boost::thread mHandlerThread;

        /**
         * The pointer of message queue
         */
		std::shared_ptr<MessageQueue> mMessageQueue = nullptr;

		/**
		 * Sensor information to be sent along with a message
		 */
		const std::shared_ptr<SensorInfo> mSensorInfo = nullptr;

        /**
         * True if the system is running
         */
        bool mOperation = true;

        /**
         * Set it for how long we will wait until the remaining job processes.
         * The unit of this number is second.
         */
        int mWaitSeconds = 2;

        /**
         * Set it for how long we will wait until the handler is destroyed.
         * The unit of this number is second.
         */
		int mDestroySeconds = 3;
		
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
		void run();

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