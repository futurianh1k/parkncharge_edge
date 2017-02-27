#include "SerialVideoReader.h"

#include <iostream>

#include <boost/date_time/local_time/local_time.hpp>

using namespace seevider;

namespace bpt = boost::posix_time;

SerialVideoReader::SerialVideoReader(int id) {
    open(id);
}

SerialVideoReader::SerialVideoReader(std::string filename) {
    open(filename);
}

SerialVideoReader::~SerialVideoReader() {
    if (mVideoCapture.isOpened()) {
        std::cout << "Close the vidao API." << std::endl;
        mVideoCapture.release();
    }
}

bool SerialVideoReader::read(cv::Mat &frame, bpt::ptime &now) {
    boost::mutex::scoped_lock lock(mMutex);
    if (!mVideoCapture.read(frame)) {
        now = bpt::second_clock::local_time();
        return false;
    }
    else {
        now = bpt::second_clock::local_time();
        return true;
    }
}

bool SerialVideoReader::isOpened() const {
    boost::mutex::scoped_lock lock(mMutex);
    return mVideoCapture.isOpened();
}

bool SerialVideoReader::open(int id) {
    boost::mutex::scoped_lock lock(mMutex);
    if (!mVideoCapture.open(id)) {
		std::cout << "Failed to connect the camera" << std::endl;
        return false;
	}

    return true;
}

bool SerialVideoReader::open(std::string filename) {
    boost::mutex::scoped_lock lock(mMutex);
    if (!mVideoCapture.open(filename)) {
		std::cout << "Failed to connect the camera" << std::endl;
        return false;
	}

    return true;
}
