#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

class IOccupancyDetector
{
public:
	IOccupancyDetector();
	~IOccupancyDetector();

	/**
	 * Detect vehicles from given image. The locations of detected vehicles
	 * will be stored to the parameter 'locs'.
	 * The number of detected vehicles will be returned.
	 */
	virtual int detect(const cv::Mat& image, std::vector<cv::Rect> &locs) = 0;
};

