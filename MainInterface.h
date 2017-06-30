// File:	MainInterface.h
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

#include "SerialVideoReader.h"
#include "ServerNetworkHandler.h"
#include "TCPSocketListener.h"
#include "ParkingSpot.h"
#include "Settings.h"
#include "IOccupancyDetector.h"

#include <boost/thread.hpp>

namespace seevider {
    /**
     * Main interface class. An instance of the class must be run within the main thread.
     * Otherwise, the application will be crashed due to the OpenCV windows.
     * The purpose of having this class is to manage various resources easily.
     */
	class MainInterface {

	private:
		/**
		 * Must be true while the system is operating.
		 */
		bool mOperation;

		/**
		 * IPv4 Address of this machine, if any
		 */
		std::string mIPv4Address;

		/**
		 * IPv6 Address of this machine, if any
		 */
		std::string mIPv6Address;

		/**
		 * Mutual condition variable to update the data from TCP socket connection.
		 */
		MutualConditionVariable mMutualConditionVariable;

		/**
		 * Settings
		 */
		std::shared_ptr<Settings> mSettings;

		/**
		 * Pointer of the video reader.
		 */
		std::shared_ptr<SerialVideoReader> mVideoReader;

		/**
		 * The massage queue for the server communication
		 */
		std::shared_ptr<MessageQueue> mServMsgQueue;

		/**
		 * The server-side network handler
		 */
		std::unique_ptr<ServerNetworkHandler> mHTTPServUploader;

		/**
		 * The manager connection handler
		 */
		std::unique_ptr<TCPSocketListener> mTCPSocketListener;

		/**
		 * Occupancy detector
		 */
		std::unique_ptr<IOccupancyDetector> mDetector;

		/**
		 * Parking spot manager
		 */
		std::shared_ptr<ParkingSpotManager> mParkingSpotManager;

		/**
		 * Window name for debugging
		 */
		cv::String mDebugWindowName = "Debugging Camera View";

		/**
		 *
		 */
		cv::String mInitializeWindow = "Parking Spot Initialization";

    public:
        MainInterface();
        ~MainInterface();

        /**
         * Main entry of the class
         */
        void run();

	private:
		/**
		 * Retrive and store network addresses of this machine
		 */
		void retrieveNetworkAddresses();

		/**
		 * Initialize parking spots. Use this function only for debugging purpose.
		 */
		void initParkingSpots();

		/**
		 * Add one parking spot
		 */
		void addParkingSpot(int id, cv::Rect roi);

		/**
		 * Remove the last parking spot
		 */
		void removeLastParkingSpot();

		/**
		 * Update occupancy status of parking spots
		 */
		void updateSpots(const cv::Mat &frame, const boost::posix_time::ptime& now);

		/**
		 * Draw current parking status
		 */
		cv::Mat drawParkingStatus(const cv::Mat& frame) const;

		/**
		 * Print the instruction to use the ROI setting window
		 */
		void print_usage_roi_settings();
    };
}
