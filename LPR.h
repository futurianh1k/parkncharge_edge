// File:	LPR.h
// Author:	Seongdo Kim
// Contact:	sdland85@gmail.com
//
// Copyright (c) 2017, Seongdo Kim <sdland85@gmail.com>
// All rights reserved.
// The copyright to the computer program(s) herein is
// the property of Seongdo Kim. The program(s) may be
// used and/or copied only with the written permission
// of Seongdo Kim or in accordance with the terms and
// conditions stipulated in the agreement/contract under
// which the program(s) have been supplied.
//
// Written by Seongdo Kim <sdland85@gmail.com>, Aug. 2017

#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

namespace seevider {
	class LPR
	{
	public:
	private:
		/**
		 * Country code, e.g., us for USA and kr for Korea
		 */
		std::string mCountryCode;

		/**
		 * License plate recognizing engine
		 */
		//alpr::Alpr mLPR;

	public:

		/**
		 * Empty constructor
		 */
		LPR();

		/**
		 * Basic constructor
		 */
		LPR(std::string country, std::string settings_filename);

		/**
		 * Recognize license plate number from given image.
		 */
		std::string recognize(const cv::Mat& image, std::vector<std::string> &LPLists = std::vector<std::string>()) const;

	private:
	};
}
