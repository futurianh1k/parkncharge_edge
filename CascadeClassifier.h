#pragma once
#include "IOccupancyDetector.h"

namespace seevider {
	class CascadeClassifier :
		public IOccupancyDetector
	{
	public:
		CascadeClassifier(std::string option_filename);
		~CascadeClassifier();

		virtual int detect(const cv::Mat &image, std::vector<cv::Rect> &locs);

	private:
		cv::CascadeClassifier mClassifier;
	};
}

