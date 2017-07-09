// File:	CameraInfo.h
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

#pragma once

#include <string>

namespace seevider {
	class CameraInfo {
	public:
		/**
		 * Desired input frame width
		 */
		const int &FrameWidth;

		/**
		 * Desired input frame height
		 */
		const int &FrameHeight;

		/**
		 * Desired input image format. Must obbey the FourCC rule, and has the length of 4.
		 */
		const std::string &FourCC;

		CameraInfo();
		~CameraInfo();

	protected:
		/**
		 * Desired input frame width
		 */
		int mFrameWidth;

		/**
		 * Desired input frame height
		 */
		int mFrameHeight;

		/**
		 * Desired input image format. Must obbey the FourCC rule, and has the length of 4.
		 */
		std::string mFourCC;
	};
}