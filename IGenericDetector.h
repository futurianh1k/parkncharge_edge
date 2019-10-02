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
<<<<<<< HEAD
	virtual int detect(const cv::Mat& image, std::vector<cv::Rect> &locs, int size) = 0;
=======
	virtual int detect(const cv::Mat& img, std::vector<cv::Rect> &locs, int size) = 0;
>>>>>>> 3441677ab1565ffd6fa168e9812f543a820cd17e

};

