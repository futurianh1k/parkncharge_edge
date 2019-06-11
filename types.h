// File:	types.h
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

#include <opencv2/opencv.hpp>

#include <boost/version.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/functional/hash.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace seevider {
	//--------------------------------
	// Parking spot related constants
	//--------------------------------

	/** 
	 * Parking spot status code
	 */
	enum PARKING_SPOT_STATUS {
		PARKING_SPOT_STATUS_EMPTY,			// Empty
		PARKING_SPOT_STATUS_OCCUPIED,		// Occupied
		PARKING_SPOT_STATUS_UNAVAILABLE,	// Spot is unavailble
		PARKING_SPOT_STATUS_USER			// User-defined status
	};

	/**
	 * Policy of each parking spot
	 */
	enum PARKING_SPOT_POLICY {
		POLICY_FORBIDDEN,		// Forbidden spot
		POLICY_TIMED,			// Timed spot.
		POLICY_UNLIMITED,		// Unlimited parkable spot
		POLICY_ACCESSIBLE,		// Accessible parking
		POLICY_DISABLED			// Distable parking spot, e.g., under construction
	};

	//--------------------------------
	// Server communication constants
	//--------------------------------

	/**
	 * Information of a server destination for each request type.
	 */
	struct ServerDestinations_t {
		/**
		 * HTTP request type
		 */
		int RequestType;

		/**
		 * Target pathes on the host side
		 */
		std::string TargetPath;

		/**
		 * HTTP request method
		 */
		std::string HTTPRequestMethod;	// Either POST or PUT
	};

	/**
	 * HTTP request types
	 */
	enum HTTP_REQ_TYPE {
		HTTP_REQ_SYNC_GENERAL,	// Synchronization request from a sensor to the server
		HTTP_REQ_UPDATE_ENTER,	// A vehicle entered to a parking spot
		HTTP_REQ_UPDATE_EXIT,	// A vehicle exited from a parking spot
		HTTP_REQ_UPDATE_OVER	// A vehicle overstayed at a parking spot
	};

	enum CLASSIFIER_TYPE {
		CLASSIFIER_CASCADE = 0,	// cascade classifier
		CLASSIFIER_CNN			// convolutional nueral network
	};

	//--------------------------
	// Arrays
	//--------------------------

	static const std::string CLASSIFIER_TYPE_STRING[] = { "cascade", "cnn" };

	//--------------------------
	// Structures
	//--------------------------

	/**
	 * Structure to suspend the receiving thread
	 */
	struct MutualConditionVariable {
		/**
		 * Abling and disabling the management mode.
		 * While this value is true, the system will not detect the vehicle and
		 * wait for receiving the update from the manager.
		 */
		bool ManagementMode = false;

		/**
		 * Conditional variable for the requesting thread
		 */
		boost::condition_variable SenderCV;

		/**
		 * Conditional variable for the thread to be suspended
		 */
		boost::condition_variable ReceiverCV;

		/**
		 * Mutex to ensure atomic operations at the entrance of the management mode
		 */
		boost::mutex MutexEntrace;

		/**
		 * Mutex to wait the update
		 */
		boost::mutex MutexExit;
	};

	/**
	 ROI Callback data to communicate with the preview window
	 */
	struct ROISettingCallbackData {
		cv::Mat image;
		cv::String window_name;
		int roi_x0 = 0;
		int roi_y0 = 0;
		int roi_x1 = 0;
		int roi_y1 = 0;
		bool start_draw = false;
		bool roi_set = false;
	};

	/**
	 * Defines custom hash functions for the use of unordered_map
	 */
	struct CustomHash {
		std::size_t operator()(const boost::posix_time::ptime& lhs) const {
			std::size_t seed = 0;
			boost::hash_combine(seed, lhs.date().day().as_number());
			boost::hash_combine(seed, lhs.time_of_day().total_seconds());
			return seed;
		}
	};

	/**
	 * Structure for saving parameters of parking sensor
	 */
	struct ParkingParams {
		int sensitivity;
		int enterCount;
		int exitCount;
	};

	/**
	 * Structure for saving parameters of lighting sensor
	 */
	struct LightingParams {
		int noiseFilterSize;
		int minMotionArea;
		int maxMotionArea;
		int lightDelayTime;
		int lightDimdownTime;
		int lightMaxLevel;
		int lightMinLevel;
	};

	/**
	 * Folder name containing the core data
	 */
	const std::string SYSTEM_FOLDER_CORE = "./core/";

	/**
	 * Folder name containing
	 */
	const std::string SYSTEM_FOLDER_LOG = "./log/";

	/**
	 * Filename to save parking spots
	 */
	const std::string SYSTEM_FILE_PARKINGSPOTS = "parkingspots.json";
	
	/**
	 * Boost major version
	 */
	#define BOOST_VERSION_MAJOR	BOOST_VERSION / 100000

	/**
	 * Boost minor version
	 */
	#define BOOST_VERSION_MINOR	BOOST_VERSION / 100 % 1000
}
