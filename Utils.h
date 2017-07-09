// File:	Utils.h
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

#pragma once

#include <string>
#include <vector>

namespace seevider {
	/**
	 * Transform string to bool
	 */
	bool to_bool(std::string str);

	/**
	 * Transform bool to string
	 */
	std::string to_string(bool val);

	/**
	 * OnMouse event handler for ROI settings
	 */
	void ROISetting_OnMouse(int event, int x, int y, int, void *data);

	/**
	 * Convert given value to readable fourCC code
	 */
	std::vector<char> cvtToFourCC(double val);
}
