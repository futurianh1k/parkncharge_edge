#pragma once

#include <opencv2/opencv.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "IMessageData.h"
#include "types.h"

namespace seevider {
    class ParkingStatus : public IMessageData {
    public:
        /**
         * Empty constructor
         */
        ParkingStatus();
        
        /**
         * Basic constructor
         */
        ParkingStatus(const int update_code, const std::string spotID,
            const cv::Mat &frame, const boost::posix_time::ptime &eventTime);

        /**
         * Basic destructor
         */
        ~ParkingStatus();

        /**
         * Construct a string to show the content of the data.
         * Generally used for debugging purpose.
         */
        virtual std::string toString() const;

        /**
        * Construct a JSON data for network communication
        */
        virtual std::string toJSONObject() const;

    private:
        /**
         * Parking spot update data
         */
        int mUpdateCode;

        /**
         * Parking spot ID
         */
        std::string mSpotID;

        /**
         * Image frame
         */
        cv::Mat mFrame;

        /**
         * Date and time that this event happened.
         */
        boost::posix_time::ptime mEventTime;
    };
}