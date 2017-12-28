#include "MotionDetection.h"

#include "Utils.h"

namespace seevider {
	using namespace cv;

	MotionDetection::MotionDetection(Size imageSize) : mTargetSize(0, 0),  mMOG2(100, 16, false) {
		if (imageSize.width > 0 && imageSize.height > 0) {
			mResize = true;
			double ratio = round(computeImageRatio(imageSize.width, imageSize.height) / 0.01);
			if (ratio == 125) {	// 1.25:1
				mTargetSize.width = 800;
				mTargetSize.height = 640;
			}
			else if (ratio == 177 || ratio == 178) {	// 16:9
				mTargetSize.width = 1024;
				mTargetSize.height = 576;
			}
			else if (ratio == 160) {	// 16:10
				mTargetSize.width = 960;
				mTargetSize.height = 600;
			}
			else {	// 1.33:1 or non-standard to 800x600
				mTargetSize.width = 800;
				mTargetSize.height = 600;
			}
		}
	}

	MotionDetection::~MotionDetection() {
	}

	void MotionDetection::update(const cv::Mat &frame) {
		if (mResize && frame.size() != mTargetSize) {
			Mat input;
			resize(frame, input, mTargetSize);
			mMOG2(input, mFGMaskMOG2);
		}
		else {
			mMOG2(frame, mFGMaskMOG2);
		}

		// Generate parameters for Open operation
		int morph_size = 3;
		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
		morphologyEx(mFGMaskMOG2, mFGMaskMOG2, MORPH_OPEN, element);	// Open process to remove minor movements
	}

	bool MotionDetection::isMotionDetected(const cv::Mat &frame, cv::Rect ROI) const {
		if (ROI.area() == 0) {	// detect motion over entire region
			int motionArea = sum(mFGMaskMOG2)[0] / 255; // normalized sum over the first channel of the motion mask

			return mMinMotionArea < motionArea && motionArea < frame.size().area() * mMaxMotionArea / 100;
		}
		else {
			if (mResize) {	// resize ROI if necessary
				double ratio_x = double(mTargetSize.width) / double(frame.cols);
				double ratio_y = double(mTargetSize.height) / double(frame.rows);
				ROI.x = cvRound(double(ROI.x) * ratio_x);
				ROI.width = cvRound(double(ROI.width) * ratio_x);
				ROI.y = cvRound(double(ROI.y) * ratio_y);
				ROI.height = cvRound(double(ROI.height) * ratio_y);
			}

			Mat maskROI = mFGMaskMOG2(ROI);
			int motionArea = sum(maskROI)[0] / 255; // normalized sum over the first channel of the motion mask

			return mMinMotionArea < motionArea && motionArea < ROI.area() * mMaxMotionArea / 100;
		}
	}
}
