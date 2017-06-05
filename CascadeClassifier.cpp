#include "CascadeClassifier.h"

namespace seevider {
	CascadeClassifier::CascadeClassifier(std::string option_filename) {
		mClassifier.load(option_filename);
	}

	CascadeClassifier::~CascadeClassifier() {
	}

	int CascadeClassifier::detect(const cv::Mat& image, std::vector<cv::Rect> &locs) {

		mClassifier.detectMultiScale(image, locs, 1.1, 2, 0, cv::Size(image.cols / 4, image.rows / 4));

		return locs.size();
	}
}
