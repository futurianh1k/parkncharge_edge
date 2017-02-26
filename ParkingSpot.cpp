#include "ParkingSpot.h"

#include <iostream>

using cv::Mat;

ParkingSpot::ParkingSpot(std::string id, const int length) :
ROI(mROI), TimeLimit(mTimeLimit), ID(mID),
ImageAtEntry(mImageAtEntry), ImageAtExit(mImageAtExit), ImageAtExpired(mImageAtExpired),
mWork(mService), mTimerThread(boost::bind(&ParkingSpot::runTimer, this)),
mParkingTimer(mService) {
    mOccupied = false;
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
    // TODO: retrive current time and put the data into message queue
    mServerMsgQueue->push(mID + ": vehicle entered");   // for test
    startTimer();
}

void ParkingSpot::expired(const cv::Mat& expiredImage) {
    //mImageAtExpired = expiredImage.clone();
    // TODO: retrive current time and put the data into message queue
    mServerMsgQueue->push(mID + ": time expired");    // for test
}

void ParkingSpot::exit(const cv::Mat& exitImage) {
    //mImageAtExit = exitImage.clone();
    mOccupied = false;
    // TODO: retrive current time and put the data into message queue
    mServerMsgQueue->push(mID + ": vehicle exited");    // for test
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

void ParkingSpot::notifyExpiration() {
    cv::Mat dummy;
    if (mOccupied) {
        std::cout << mID << " was expired!" << std::endl;   // for test
        expired(dummy);
    }
    else {
        std::cout << mID << " was exited!" << std::endl;   // for test
        //exit(dummy);
    }
}

//-----------------------------------
// Static functions and variables
//-----------------------------------

MessageQueue<std::string> *ParkingSpot::mServerMsgQueue = NULL;

void ParkingSpot::setMessageQueue(MessageQueue<std::string> *messageQueue) {
    mServerMsgQueue = messageQueue;
}
