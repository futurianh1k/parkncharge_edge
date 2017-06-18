// File:	IMessageData.h
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

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace seevider {
    /**
    * Interface class of message data
    * All message data for MessageQueue must be derived from this class.
    */
    class IMessageData {
	public:
		IMessageData();

		IMessageData(boost::posix_time::ptime eventTime);

        /**
         * Construct a string to show the content of the data
         */
        virtual std::string toString() const = 0;

        /**
         * Construct a JSON data for network communication
         */
		virtual boost::property_tree::ptree toPTree() const = 0;

		/**
		 * Save date to given folder
		 */
		virtual bool save(const std::string folder) const = 0;

		/**
		 * Load the oldest data from given folder
		 */
		virtual bool load(const std::string folder) = 0;

	protected:
		/**
		 * Date and time that this event happened.
		 */
		boost::posix_time::ptime mEventTime;
    };
}
