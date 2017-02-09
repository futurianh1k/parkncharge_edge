#include "ParkingSpot.h"

#include <iostream>

using cv::Mat;

ParkingSpot::ParkingSpot(std::string id, const int length) :
ROI(mROI), TimeLimit(mTimeLimit), ID(mID),
ImageAtEntry(mImageAtEntry), ImageAtExit(mImageAtExit), ImageAtExpired(mImageAtExpired),
mWork(mService), mTimerThread(boost::bind(&ParkingSpot::runTimer, this)),
mParkingTimer(mService) {
    mTimeLimit = length;
    mID = id;
}

ParkingSpot::~ParkingSpot() {
    mService.stop();
    mTimerThread.join();
}

bool ParkingSpot::isOccupied() {
    return mOccupied;
}

void ParkingSpot::enter(const cv::Mat& entryImage) {
    //mImageAtEntry = entryImage.clone();
    mOccupied = true;
    startTimer();
}

void ParkingSpot::expired(const cv::Mat& expiredImage) {
    mImageAtExpired = expiredImage.clone();
}

void ParkingSpot::exit(const cv::Mat& exitImage) {
    //mImageAtExit = exitImage.clone();
    mOccupied = false;
    stopTimer();
}

void ParkingSpot::runTimer() {
    mService.run();
}

void ParkingSpot::startTimer() {
    mParkingTimer.expires_from_now(boost::posix_time::seconds(mTimeLimit));
    mParkingTimer.async_wait(boost::bind(&ParkingSpot::notifyExpiration, this));
}

void ParkingSpot::stopTimer() {
    mParkingTimer.cancel();
}

void ParkingSpot::notifyExpiration() const {
    std::cout << mID << " was expired!" << std::endl;
}