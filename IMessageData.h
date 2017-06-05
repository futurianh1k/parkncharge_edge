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
		virtual bool save(std::string folder) const = 0;

		/**
		 * Load the oldest data from given folder
		 */
		virtual bool load(std::string folder) = 0;

	protected:
		/**
		 * Date and time that this event happened.
		 */
		boost::posix_time::ptime mEventTime;
    };
}
