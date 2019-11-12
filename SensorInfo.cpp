// File:	SensorInfo.cpp
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

#include "SensorInfo.h"
#include <iostream>
namespace seevider {
	SensorInfo::SensorInfo() :
//		SensorID(mSensorID), SensorName(mSensorName), TimeZone(mTimeZone), ParkingParams(mParkingParams), LightingParams(mLightingParams) {
		SensorID(mSensorID), SensorName(mSensorName), TimeZone(mTimeZone), ParkingParams(mParkingParams) {
		//mSensorID = sensorID;
		std::cout << std::endl << "----------<<< SensorInfo.cpp in  >>>----------" << std::endl << std::endl;
	};

	SensorInfo::~SensorInfo() {
	}
}
