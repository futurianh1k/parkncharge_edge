// File:	ParkingSpot.cpp
// Author:	Seongdo Kim
// Contact:	sdland85@gmail.com
//
// Copyright (c) 2017, Seongdo Kim <sdland85@gmail.com>
// All rights reserved.
// The copyright to the computer program(s) herein is
// the property of Seongdo Kim. The program(s) may be
// used and/or copied only with the written permission
// of Seongdo Kim or in accordance with the terms and
// conditions stipulated in the agreement/contract under
// which the program(s) have been supplied.
//
// Written by Seongdo Kim <sdland85@gmail.com>, June, 2017
//

#include "ParkingSpot.h"
#include "types.h"
#include "ParkingUpdateMessage.h"

#include <glog/logging.h>

using namespace seevider;
namespace pt = boost::posix_time;

using cv::Mat;

ParkingSpot::ParkingSpot(int id, std::string spotName, const int length,
	const cv::Rect roi, PARKING_SPOT_POLICY policy) : ID(mID), SpotName(mSpotName),
	ROI(mROI), TimeLimit(mTimeLimit), ParkingPolicy(mParkingPolicy),
	UpdateEnabled(mUpdateEnabled),
	mID(id), mSpotName(spotName), mTimeLimit(length), mROI(roi),
	mParkingPolicy(policy), mOccupied(false), mOverstayed(false), mUpdateEnabled(true),
	mWork(mService), mTimerThread(boost::bind(&ParkingSpot::runTimer, this)),
	mParkingTimer(mService) {
}

ParkingSpot::~ParkingSpot() {
    mService.stop();
    mTimerThread.join();

	DLOG(INFO) << "Release parking spot " << mID;
}

bool ParkingSpot::isOccupied() const {
    return mOccupied;
}

bool ParkingSpot::isOverstayed() const {
	return mOccupied && mOverstayed;
}

void ParkingSpot::enter(const Mat& entryImage, const pt::ptime &entryTime, const std::string PN) {
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_ENTER, mID, entryImage, entryTime, PN);

	mOccupied = true;
    mServerMsgQueue->push(data);
	
	mEntryTime = entryTime;

	startTimer();
	LOG(INFO) << "Parking spot ID " << mID << " has occupied at " << to_simple_string(entryTime);
}

void ParkingSpot::overstayed(const Mat& expiredImage, const pt::ptime &exprTime) {
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_OVER, mID, expiredImage.clone(), exprTime);
	mOverstayed = true;
	mServerMsgQueue->push(data);
}

void ParkingSpot::exit(const Mat& exitImage, const pt::ptime &exitTime) {
	std::unique_ptr<IMessageData> data = std::make_unique<ParkingUpdateMessage>(
		HTTP_REQ_UPDATE_EXIT, mID, exitImage, exitTime);

    mOccupied = false;
	mOverstayed = false;
	mServerMsgQueue->push(data);

	stopTimer();
	LOG(INFO) << "Parking spot ID " << mID << " has released at " << to_simple_string(exitTime);
}

bool ParkingSpot::update(std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy) {
	if (mOccupied && !mOverstayed) {
		LOG(WARNING) << "Failed to update parking spot id " << mID << ". To update a parking spot, parking timer must be disabled.";
		return false;
	}

	mSpotName = spotName;
	mTimeLimit = timeLimit;
	mROI = roi;
	mParkingPolicy = policy;

	return true;
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

void ParkingSpot::reset() {
	stopTimer();
	mOccupied = false;
	mOverstayed = false;
	mUpdateEnabled = true;
	mOccupiedFrameCounter = 0;
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

		LOG(INFO) << "Parking spot ID " << mID << " has expired.";
		overstayed(frame, time);
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