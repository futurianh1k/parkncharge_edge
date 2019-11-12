// File:	CameraInfo.cpp
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
// Written by Seongdo Kim <sdland85@gmail.com>, July, 2017
//

#include "CameraInfo.h"
#include <iostream>
namespace seevider {
	CameraInfo::CameraInfo() :
		FrameWidth(mFrameWidth), FrameHeight(mFrameHeight), FourCC(mFourCC) {
		std::cout << std::endl << "----------<<< CameraInfo.cpp in  >>>----------" << std::endl << std::endl;
	}

	CameraInfo::~CameraInfo() {
	}
}
