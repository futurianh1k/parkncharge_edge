#pragma once

#include <opencv2/opencv.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "IMessageData.h"
#include "types.h"

namespace seevider {
	class ParkingUpdateMessage : public IMessageData {
	public:
		/**
		 * Basic constructor
		 */
		ParkingUpdateMessage();

        /**
         * Constructor with initialization
         */
		ParkingUpdateMessage(const int request, const int spotID,
            const cv::Mat &frame, const boost::posix_time::ptime &eventTime);

        /**
         * Basic destructor
         */
		~ParkingUpdateMessage();

        /**
         * Construct a string to show the content of the data.
         * Generally used for debugging purpose.
         */
		virtual std::string toString() const override;

        /**
        * Construct a JSON data for network communication
        */
		virtual boost::property_tree::ptree toPTree() const override;

		/**
		 * Save date to given folder
		 */
		virtual bool save(const std::string folder) const override;

		/**
		 * Load the oldest data from given folder
		 */
		virtual bool load(const std::string filename) override;

    private:
        /**
         * Parking spot update data
         */
        int mRequestCode;

        /**
         * Local parking spot ID
         */
        int mSpotID;

        /**
         * Image frame
         */
        cv::Mat mFrame;

		/**
		 * Construct a filename
		 */
		cv::String constructFilename() const;
    };
}