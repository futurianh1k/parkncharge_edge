#pragma once

#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace seevider {
    class SerialVideoReader {
    public:
        /**
         * Construct the class instance and set the input source as a camera.
         */
        SerialVideoReader(int id = 0);

        /**
         * Construct the class instance and set the input source to a video file.
         */
        SerialVideoReader(std::string filename);

        /**
         * Basic destructor. If the video API is still opened, close it.
         */
        ~SerialVideoReader();

        /**
         * Retrieve a frame from the input source.
         * @returns True if success, and false otherwise.
         */
        bool read(cv::Mat &frame, boost::posix_time::ptime &now);

        /**
         * Check if video API is currently opened.
         * @returns True if API is opened, and false otherwise.
         */
        bool isOpened() const;

        /**
         * Open a video API from one of connected cameras.
         * @params id An ID of connected camera.
         * @returns True if success, and false otherwise.
         */
        bool open(int id);

        /**
         * Open a video API from given file.
         * @params filename A filename of target video. The file must be a vaild video file.
         * @returns True if success, and false otherwise.
         */
        bool open(std::string filename);

    private:
        /**
         * Instance of a video source API.
         */
        cv::VideoCapture mVideoCapture;

        /**
         * A mutex to limit the simultaneous access to the video API.
         */
        mutable boost::mutex mMutex;
    };
}