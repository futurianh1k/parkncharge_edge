#pragma once

#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

class IGenericDetector
{
public:
	IGenericDetector();
	~IGenericDetector();

	/**
	 * Detect objects from given image. The locations of detected objects
	 * will be stored to the parameter 'locs'.
	 * The number of detected objects will be returned.
	 */
	virtual int detect(const cv::Mat& image, std::vector<cv::Rect> &locs, int size) = 0;

};

