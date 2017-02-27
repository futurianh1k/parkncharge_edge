#pragma once

#include <string>

namespace seevider {
    /**
    * Interface class of message data
    * All message data for MessageQueue must be derived from this class.
    */
    class IMessageData {
        /**
        * Construct a string to show the content of the data
        */
        virtual std::string toString() const = 0;

        /**
        * Construct a JSON data for network communication
        */
        virtual std::string toJSONObject() const = 0;
    };
}