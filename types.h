#pragma once

#include <opencv2/opencv.hpp>
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
	 * JSON request key
	 */
	const std::string JSON_KEY_REQUEST = "request";

	const int JSON_REQUEST_STREAMING = 1;
	const int JSON_REQUEST_DISCONNECT = 9;
}
