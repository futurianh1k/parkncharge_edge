#include "ParkingSpot.h"

#include <iostream>

#include "types.h"

using namespace seevider;
namespace pt = boost::posix_time;

using cv::Mat;

ParkingSpot::ParkingSpot(std::string id, const int length) :
ROI(mROI), TimeLimit(mTimeLimit), ID(mID),
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

bool ParkingSpot::isOccupied() const {
    return mOccupied;
}

void ParkingSpot::enter(const Mat& entryImage, const pt::ptime &entryTime) {
    mOccupied = true;
    mServerMsgQueue->push(ParkingStatus(
        (int)PARKING_SPOT_UPDATE::PARKING_SPOT_UPDATE_ENTER,
        mID, entryImage.clone(), entryTime));

    startTimer();
}

void ParkingSpot::expired(const Mat& expiredImage, const pt::ptime &exprTime) {
    mServerMsgQueue->push(ParkingStatus(
        (int)PARKING_SPOT_UPDATE::PARKING_SPOT_UPDATE_EXPIRED,
        mID, expiredImage.clone(), exprTime));
}

void ParkingSpot::exit(const Mat& exitImage, const pt::ptime &exitTime) {
    mOccupied = false;
    mServerMsgQueue->push(ParkingStatus(
        (int)PARKING_SPOT_UPDATE::PARKING_SPOT_UPDATE_EXIT,
        mID, exitImage.clone(), exitTime));

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
    if (mOccupied) {
        Mat frame;
        pt::ptime time;

        mVideoReader->read(frame, time);

        std::cout << mID << " was expired!" << std::endl;   // for test
        expired(frame, time);
    }
}

//-----------------------------------
// Static functions
//-----------------------------------

void ParkingSpot::setMessageQueue(MessageQueue<ParkingStatus> *messageQueue) {
    mServerMsgQueue = messageQueue;
}

void ParkingSpot::setVideoReader(SerialVideoReader *videoReader) {
    mVideoReader = videoReader;
}

//-----------------------------------
// Static variables
//-----------------------------------

MessageQueue<ParkingStatus> *ParkingSpot::mServerMsgQueue = NULL;
SerialVideoReader *ParkingSpot::mVideoReader = NULL;