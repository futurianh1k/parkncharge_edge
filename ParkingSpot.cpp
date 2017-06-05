#include "ParkingSpot.h"

#include <iostream>

#include "types.h"
#include "ParkingUpdateMessage.h"

using namespace seevider;
namespace pt = boost::posix_time;

using cv::Mat;

ParkingSpot::ParkingSpot(int id, std::string spotName, const int length,
	const cv::Rect roi, PARKING_SPOT_POLICY policy) : ID(mID), SpotName(mSpotName),
	ROI(mROI), TimeLimit(mTimeLimit), ParkingPolicy(mParkingPolicy),
	UpdateEnabled(mUpdateEnabled),
	mID(id), mSpotName(spotName), mTimeLimit(length), mROI(roi),
	mParkingPolicy(policy),	mOccupied(false), mUpdateEnabled(true),
	mWork(mService), mTimerThread(boost::bind(&ParkingSpot::runTimer, this)),
	mParkingTimer(mService) {
}

ParkingSpot::~ParkingSpot() {
    mService.stop();
    mTimerThread.join();

	std::cout << "Release parking spot " << mID << std::endl;
}

bool ParkingSpot::isOccupied() const {
    return mOccupied;
}

void ParkingSpot::enter(const Mat& entryImage, const pt::ptime &entryTime) {
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_ENTER, mID, entryImage, entryTime);

	mOccupied = true;
    mServerMsgQueue->push(data);
	
	mEntryTime = entryTime;

	std::cout << "Timer of ID " << mID << "starts at " << to_simple_string(entryTime) << std::endl;
    startTimer();
}

void ParkingSpot::expired(const Mat& expiredImage, const pt::ptime &exprTime) {
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_OVER, mID, expiredImage.clone(), exprTime);

	mServerMsgQueue->push(data);
}

void ParkingSpot::exit(const Mat& exitImage, const pt::ptime &exitTime) {
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_EXIT, mID, exitImage, exitTime);

    mOccupied = false;
	mServerMsgQueue->push(data);

    stopTimer();
}

bool ParkingSpot::update(bool occupied, bool triggerUpdatability) {
	bool updated = false;

	if (occupied) {
		if (mOccupiedFrameCounter < mPositiveThreshold) {
			mOccupiedFrameCounter++;
		}
		else if (mOccupiedFrameCounter - mPositiveThreshold < mPositiveThreshold) {
			if (!mOccupied) {
				mOccupied = true;
				updated = true;
			}

			mOccupiedFrameCounter++;
		}
		else if (mOccupiedFrameCounter - mPositiveThreshold >= mPositiveThreshold && triggerUpdatability) {
			mUpdateEnabled = false;
		}
	}
	else {
		if (mOccupiedFrameCounter > mNegativeThreshold) {
			mOccupiedFrameCounter--;
		}
		else if (mOccupiedFrameCounter - mNegativeThreshold > mNegativeThreshold) {
			if (mOccupied) {
				mOccupied = false;
				updated = true;
			}

			mOccupiedFrameCounter--;
		}
		else if (mOccupiedFrameCounter - mNegativeThreshold <= mNegativeThreshold && triggerUpdatability) {
			mUpdateEnabled = false;
		}
	}

	return updated;
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
		pt::ptime time = mEntryTime + boost::posix_time::seconds(mTimeLimit);

        mVideoReader->readAt(frame, time);

        std::cout << mID << " was expired!" << std::endl;   // for test
        expired(frame, time);
    }
}

//-----------------------------------
// Static functions
//-----------------------------------

void ParkingSpot::setMessageQueue(std::shared_ptr<MessageQueue> &messageQueue) {
    mServerMsgQueue = messageQueue;
}

void ParkingSpot::setVideoReader(std::shared_ptr<SerialVideoReader> &videoReader) {
    mVideoReader = videoReader;
}

void ParkingSpot::setPositiveThreshold(int positiveThreshold) {
	mPositiveThreshold = positiveThreshold;
}

void ParkingSpot::setNegativeThreshold(int negativeThreshold) {
	mNegativeThreshold = negativeThreshold;
}

//-----------------------------------
// Static variables
//-----------------------------------

std::shared_ptr<MessageQueue> ParkingSpot::mServerMsgQueue = nullptr;
std::shared_ptr<SerialVideoReader> ParkingSpot::mVideoReader = nullptr;
int ParkingSpot::mPositiveThreshold = 5;
int ParkingSpot::mNegativeThreshold = -3;