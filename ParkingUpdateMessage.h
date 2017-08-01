// File:	ParkingUpdateMessage.h
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
            const cv::Mat &frame, const boost::posix_time::ptime &eventTime,
			const std::string PN = "null");

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
        int mRequestCode = 0;

        /**
         * Local parking spot ID
         */
        int mSpotID = 0;

        /**
         * Image frame
         */
        cv::Mat mFrame;

		/**
		 * Plate number, if any
		 */
		std::string mPN = "null";

		/**
		 * Construct a filename
		 */
		cv::String constructFilename() const;
    };
}