// File:	CascadeClassifier.cpp
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
// Written by Seongdo Kim <sdland85@gmail.com>, June, 2017
//

#include <logging.h>
#include "CascadeClassifier.h"

namespace seevider {
	CascadeClassifier::CascadeClassifier(std::string option_filename, std::shared_ptr<Settings> &settings) {
		mClassifier.load(option_filename);
		mSettings = settings;
	}

	CascadeClassifier::~CascadeClassifier() {
	}

	int CascadeClassifier::detect(const cv::Mat& img, std::vector<cv::Rect> &locs, int size) {
        mClassifier.detectMultiScale(img, locs, 1.1, mSettings->ParkingParams.sensitivity, 0, cv::Size(img.cols / size, img.rows / size));

		return locs.size();
	}
}
