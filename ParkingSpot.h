#pragma once

#include <string>
#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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
     * The read-only image which is taken at the time of entry.
     */
    const cv::Mat& ImageAtEntry;

    /**
     * The read-only image which is taken at the time of exit.
     */
    const cv::Mat& ImageAtExit;

    /**
     * The read-only image which is taken at the expiration time.
     */
    const cv::Mat& ImageAtExpired;

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
     bool isOccupied();

     /**
      * Must be called when the vehicle comes in.
      */
     void enter(const cv::Mat& entryImage);

     /**
      * Must be called when the maximum allowed time has reached.
      */
     void expired(const cv::Mat& expiredImage);

     /**
      * Must be called when the vehicle goes out.
      */
     void exit(const cv::Mat& exitImage);

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
     * The original image taken at the time of entry
     */
    cv::Mat mImageAtEntry;

    /**
     * The original image taken at the time of exit.
     */
    cv::Mat mImageAtExit;

    /**
     * The read-only image which is taken at the expiration time.
     */
    cv::Mat mImageAtExpired;

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
    void notifyExpiration() const;
};