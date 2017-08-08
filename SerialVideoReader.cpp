// File:	SerialVideoReader.cpp
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

#include "SerialVideoReader.h"
#include "Utils.h"

#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <glog/logging.h>

using namespace seevider;

namespace bpt = boost::posix_time;

SerialVideoReader::SerialVideoReader(std::shared_ptr<CameraInfo> setting) :
mThread(boost::bind(&SerialVideoReader::run, this)) {
	mFrameSize.width = setting->FrameWidth;
	mFrameSize.height = setting->FrameHeight;
	setting->FourCC.copy(mInputFourCC, 4, 0);
}

SerialVideoReader::~SerialVideoReader() {
	if (mThread.joinable()) {
		// If the thread is still joinable, destroy it.
		destroy();
	}
}

bool SerialVideoReader::open(int id) {
	if (mVideoReader.isOpened()) {
		LOG(WARNING) << "The video reader is already opened! It must be closed before open new one";
		return false;
	}

	if (!mVideoReader.open(id)) {
		LOG(FATAL) << "Failed to connect the camera";
		return false;
	}

	Operation = true;
	mFromVideo = false;

	// Retrieve and print the current attributes
	std::vector<char> EXT;
	cv::Size originalSize;
	double ext_code = mVideoReader.get(CV_CAP_PROP_FOURCC);
	if (ext_code >= 0.0) {
		EXT = cvtToFourCC(ext_code);
		LOG(INFO) << "The original camera codec: " << EXT[0] << EXT[1] << EXT[2] << EXT[3];
	}
	else {
		LOG(ERROR) << "Failed to retrieve camera codec: " << ext_code;
	}
	originalSize.width = (int)mVideoReader.get(CV_CAP_PROP_FRAME_WIDTH);
	originalSize.height = (int)mVideoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
	LOG(INFO) << "The original camera frame size: " << originalSize;

	// Modify video properties
	LOG(INFO) << "Modifying camera attributes";
	if (!mVideoReader.set(CV_CAP_PROP_FOURCC,
		CV_FOURCC(mInputFourCC[0], mInputFourCC[1], mInputFourCC[2], mInputFourCC[3]))) {
		LOG(ERROR) << "Failed to change the input frame format: " << mInputFourCC;
	}
	else {
		EXT = cvtToFourCC(mVideoReader.get(CV_CAP_PROP_FOURCC));
		LOG(INFO) << "Modified camera codec: " << EXT[0] << EXT[1] << EXT[2] << EXT[3];
	}
	if (mFrameSize.width > 0 && mFrameSize.height > 0) {
		if (!mVideoReader.set(CV_CAP_PROP_FRAME_WIDTH, mFrameSize.width) ||
			!mVideoReader.set(CV_CAP_PROP_FRAME_HEIGHT, mFrameSize.height)) {
			LOG(ERROR) << "Failed to change the input frame size: " << mFrameSize;
			mFrameSize = originalSize;
		}
	}
	else {
		LOG(INFO) << "Modified camera frame size: " << (int)mVideoReader.get(CV_CAP_PROP_FRAME_WIDTH) << ", " << (int)mVideoReader.get(CV_CAP_PROP_FRAME_HEIGHT);
	}

	mReady = true;

	return true;
}

bool SerialVideoReader::open(std::string filename) {
	if (mVideoReader.isOpened()) {
		LOG(WARNING) << "The video reader is already opened! It must be closed before open new one";
		return false;
	}

	if (!mVideoReader.open(filename)) {
		LOG(FATAL) << "Failed to connect the camera";
		return false;
	}

	Operation = true;
	mFromVideo = true;

	mFrameSize = cv::Size((int)mVideoReader.get(CV_CAP_PROP_FRAME_WIDTH), (int)mVideoReader.get(CV_CAP_PROP_FRAME_HEIGHT));

	return true;
}

void SerialVideoReader::destroy() {
	LOG(INFO) << "Destroying the video input handler";

	Operation = false;
	mThread.try_join_for(boost::chrono::seconds(mWaitSeconds));
	if (mThread.joinable()) {
		// If the thread is still joinable, force to cancel it.
		mThread.interrupt();
		mThread.try_join_for(boost::chrono::seconds(mDestroySeconds));
	}

	LOG(INFO) << "The video input handler has destroyed.";
}

cv::Mat SerialVideoReader::read() const {
	boost::mutex::scoped_lock lock(mMutex);

	std::pair<boost::posix_time::ptime, cv::Mat> data = mFrameQueue.back();

	return data.second.clone();
}

bool SerialVideoReader::read(cv::Mat &frame, bpt::ptime &now) {
    boost::mutex::scoped_lock lock(mMutex);

	if (mFrameQueue.empty()) {
		return false;
	}

	std::pair<boost::posix_time::ptime, cv::Mat> data = mFrameQueue.back();

	now = data.first;
	data.second.copyTo(frame);

	return true;
}

bool SerialVideoReader::readAt(cv::Mat &frame, boost::posix_time::ptime &time) {
	boost::mutex::scoped_lock lock(mMutex);

	if (mFrameIndexer.empty()) {
		return false;
	}

	LOG(INFO) << "Frame has requestd at " << to_simple_string(time);

	// Check at most ten seconds to find a frame of the closest moment
	for (int i = 0; i <= 10; i++) {
		boost::posix_time::ptime frameTime = time + boost::posix_time::seconds(i);
		if (mFrameIndexer.find(frameTime) != mFrameIndexer.end()) {
			mFrameIndexer[frameTime].copyTo(frame);

			LOG(INFO) << "Frame has found at " << to_simple_string(frameTime);

			return true;
		}
	}

	return false;
}

bool SerialVideoReader::isOpened() const {
    boost::mutex::scoped_lock lock(mMutex);
	return mVideoReader.isOpened();
}

bool SerialVideoReader::isReady() const {
	boost::mutex::scoped_lock lock(mMutex);
	
	return !(mFrameQueue.empty() || mFrameIndexer.empty()) && !mFrameQueue.back().second.empty() && mReady;
}

void SerialVideoReader::close() {
	Operation = false;

	destroy();

	if (mVideoReader.isOpened()) {
		LOG(INFO) << "Closing the vidao API";
		mVideoReader.release();
	}
}

cv::Size SerialVideoReader::size() const {
	return mFrameSize;
}

void SerialVideoReader::run() {
    bpt::ptime prev = bpt::second_clock::local_time();
	bpt::ptime frontOfIndexerTime;

    // Wait and check if video input is opened. May need to be improved.
    while (!(isOpened() && mReady)) {
		boost::this_thread::sleep_for(boost::chrono::seconds(1));
    }

    // Only this thread will access the instance of the actual VideoCapture.
	while (Operation) {
        cv::Mat frame;

		mVideoReader.read(frame);   // read one frame
        bpt::ptime now = bpt::second_clock::local_time();   // time stamp
		
        // Insert the acquired frame to the queue
		if (mFrameQueue.size() < mMaxFrames) {
			boost::mutex::scoped_lock lock(mMutex);

			mFrameQueue.push_back({ now, frame });
		}
		else {
			boost::mutex::scoped_lock lock(mMutex);
			
			mFrameQueue.pop_front();
			mFrameQueue.push_back({ now, frame });
		}

        // mFrameIndexer holds the frame of each second, back to given time period.
		if (mFrameIndexer.empty()) {
			boost::mutex::scoped_lock lock(mMutex);

			frontOfIndexerTime = now;
			mFrameIndexer.insert({ now, frame });
		}
		else {
			boost::mutex::scoped_lock lock(mMutex);

			if (mFrameIndexer.size() >= mHoldingFramesSeconds) {
				while (mFrameIndexer.find(frontOfIndexerTime) == mFrameIndexer.end()) {
					frontOfIndexerTime = frontOfIndexerTime + boost::posix_time::seconds(1);
				}
				mFrameIndexer.erase(frontOfIndexerTime);
			}

			bpt::time_duration diff = now - prev;
			if (diff.total_seconds() == 1) {
				mFrameIndexer.insert({ now, frame });
			}
		}

        prev = now;

		if (mFromVideo) {
			boost::this_thread::sleep_for(boost::chrono::microseconds(30));
		}
    }
}
