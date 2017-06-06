#include "SerialVideoReader.h"

#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <glog/logging.h>

using namespace seevider;

namespace bpt = boost::posix_time;

SerialVideoReader::SerialVideoReader() :
mThread(boost::bind(&SerialVideoReader::run, this)) {
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

	//VideoReader.set(cv::CAP_PROP_AUTOFOCUS, 1);

	mFrameSize = cv::Size((int)mVideoReader.get(CV_CAP_PROP_FRAME_WIDTH), (int)mVideoReader.get(CV_CAP_PROP_FRAME_HEIGHT));

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

	return !(mFrameQueue.empty() || mFrameIndexer.empty());
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
    while (!isOpened()) {
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

        // Insert the frame to the indexer only if this frame is the first frame for the current time
		if (mFrameIndexer.empty()) {
			boost::mutex::scoped_lock lock(mMutex);

			frontOfIndexerTime = now;
			mFrameIndexer.insert({ now, frame });
		}
		else {
			boost::mutex::scoped_lock lock(mMutex);

			if (mFrameIndexer.size() >= mMaxFrames) {
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
			boost::this_thread::sleep_for(boost::chrono::microseconds(40));
		}
    }
}
