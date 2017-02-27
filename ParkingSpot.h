#pragma once

#include <string>
#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "MessageQueue.h"
#include "SerialVideoReader.h"
#include "ParkingStatus.h"

namespace seevider {
    class ParkingSpot {
    public:
        
        /**
         * The read-only rectangular ROI
         */
        const cv::Rect& ROI;

        /**
         * The read-only time limit as minutes
         */
        const int& TimeLimit;
        
        /**
         * The read-only server-synchronized ID of the parking spot
         */
        const std::string& ID;

        /**
         * Basic constructor.
         * @params entryImage The original image taken at the time of entry.
         *  Only a clone of the image will be stored.
         * @param length Allowed length of time for this parking spot
         */
        ParkingSpot(std::string id, const int length);

        ~ParkingSpot();

        /**
         * Check if the parking spot is occupied
         */
        bool isOccupied() const;

        /**
         * Must be called when the vehicle comes in.
         */
        void enter(const cv::Mat& entryImage, const boost::posix_time::ptime &entryTime);

        /**
         * Must be called when the maximum allowed time has reached.
         */
        void expired(const cv::Mat& expiredImage, const boost::posix_time::ptime &exprTime);

        /**
         * Must be called when the vehicle goes out.
         */
        void exit(const cv::Mat& exitImage, const boost::posix_time::ptime &exitTime);

    private:
        /**
         * The original rectangular ROI
         */
        cv::Rect mROI;

        /**
         * Time limit as minute
         */
        int mTimeLimit;

        /**
         * The read-only server-synchronized ID of the parking spot
         */
        std::string mID;

        /**
         * Must be true if the parking spot is occupied
         */
        bool mOccupied;

        /**
         * Designate if the entry image is uploaded to the server.
         */
        bool mIsEntryImageUploaded;

        /**
         * Designate if the exit image is uploaded to the server.
         */
        bool mIsExitImageUploaded;

        /**
         * IO Service object for the deadline timer
         */
        boost::asio::io_service mService;

        /**
         * Work object for the deadline timer
         */
        boost::asio::io_service::work mWork;

        /**
         * Timer thread
         */
        boost::thread mTimerThread;

        /**
         * Timer to check if the parking expires
         */
        boost::asio::deadline_timer mParkingTimer;

        /**
         * Main entry of the timer thread
         */
        void runTimer();

        /**
         * Start the parking timer
         */
        void startTimer();

        /**
         * Stop the parking timer, if any
         */
        void stopTimer();

        /**
         * Notify a expiration message to the global message queue
         */
        void notifyExpiration();

        //-----------------------------------
        // Static functions and variables
        //-----------------------------------
    public:
        /**
         * Set the shared message queue for server-side communication.
         */
        static void setMessageQueue(MessageQueue<ParkingStatus> *messageQueue);

        /**
         * Set the camera frame reader for the expiration event.
         */
        static void setVideoReader(SerialVideoReader *videoReader);

    private:
        /**
         * Network message queue for uploading the parking spot status information to the server.
         * Data will be inserted when the status changes
         */
        static MessageQueue<ParkingStatus> *mServerMsgQueue;

        /**
         * Since the parking enforcement is time-critical, each timer needs to acquire the camera
         * frame at the time of an expiration event.
         */
        static SerialVideoReader *mVideoReader;
    };
}