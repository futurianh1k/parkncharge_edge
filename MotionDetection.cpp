#include "MotionDetection.h"

#include "Utils.h"
#include <glog/logging.h>


namespace seevider {
	using namespace cv;

    //Ken fix opencv3
    //Mat mFGMaskMOG2;
    Ptr<BackgroundSubtractor> mMOG2 = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();
    //Ptr<BackgroundSubtractor> mMOG2 = createBackgroundSubtractorMOG2(100, 16, false);

	MotionDetection::MotionDetection(Size imageSize) : mTargetSize(0, 0), mMOG2() {
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
        // create foreground mask of proper size
        if( mFGMaskMOG2.empty() ){
            mFGMaskMOG2.create(frame.size(), frame.type());
        }
		#if 1
		if (mResize && frame.size() != mTargetSize) {
			Mat input;
            LOG(INFO) << "checkpoint b1";
			resize(frame, input, mTargetSize);
            LOG(INFO) << "checkpoint b2";
			mMOG2->apply(input, mFGMaskMOG2, true ? -1 : 0);
            LOG(INFO) << "checkpoint b3";
		}
		else 
		#endif
		{
            LOG(INFO) << "checkpoint b4";
			mMOG2->apply(frame, mFGMaskMOG2, true ? -1 : 0);
		}

		// Generate parameters for Open operation
		int morph_size = 3;
        LOG(INFO) << "checkpoint b5";
		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
        LOG(INFO) << "checkpoint b6";
		morphologyEx(mFGMaskMOG2, mFGMaskMOG2, MORPH_OPEN, element);	// Open process to remove minor movements
		LOG(INFO) << "checkpoint b7";
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
