#include "ParkingSpot.h"

using cv::Mat;

ParkingSpot::ParkingSpot(const int length) :
ROI(mROI), TimeLimit(mTimeLimit), ID(mID),
ImageAtEntry(mImageAtEntry), ImageAtExit(mImageAtExit), ImageAtExpired(mImageAtExpired) {
    mTimeLimit = length;
}

bool ParkingSpot::isOccupied() {
    return mOccupied;
}

void ParkingSpot::enter(const cv::Mat& entryImage) {
    mImageAtEntry = entryImage.clone();
    mOccupied = true;
}

void ParkingSpot::expired(const cv::Mat& expiredImage) {
    mImageAtExpired = expiredImage.clone();
}

void ParkingSpot::exit(const cv::Mat& exitImage) {
    mImageAtExit = exitImage.clone();
    mOccupied = false;
}
