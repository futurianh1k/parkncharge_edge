#pragma once

#include <opencv2/opencv.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "IMessageData.h"
#include "types.h"

namespace seevider {
    class ParkingStatus : public IMessageData {
    public:
        /**
         * Basic constructor
         */
        ParkingStatus(const int update_code, const int spotID,
            const cv::Mat &frame, boost::posix_time::ptime eventTime);

        /**
         * Basic destructor
         */
        ~ParkingStatus();

        /**
         * Construct a string to show the content of the data.
         * Generally used for debugging purpose.
         */
        std::string toString() const;

        /**
        * Construct a JSON data for network communication
        */
        std::string toJSONObject() const;

    private:
        /**
         * Parking spot update data
         */
        int mUpdateCode;

        /**
         * Parking spot ID
         */
        int mSpotID;

        /**
         * Image frame
         */
        cv::Mat mFrame;

        /**
         * Date and time that this event happened.
         */
        boost::posix_time::ptime mTime;
    };
}