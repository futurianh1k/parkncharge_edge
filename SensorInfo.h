// File:	SensorInfo.h
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
#include "CameraInfo.h"
#include "SerialVideoReader.h"

namespace seevider {
	class SensorInfo {
	public:
		/**
		 * The read-only sensor identification string
		 */
		const std::string &SensorID;

		/**
		 * The read-only user-friendly sensor name
		 */
		const std::string &SensorName;

		/**
		 * The read-only time-zone string, e.g., [+|-]##
		 */
		const std::string &TimeZone;

        /**
         * The read-only parameters of parking sensor
         */
        const seevider::ParkingParams &ParkingParams;

        /**
         * The read-only parameters of lighting sensor
         */
        const seevider::LightingParams &LightingParams;

		/**
		 * Basic constructor
		 */
		SensorInfo();

		/**
		 * Basic destructor
		 */
		~SensorInfo();

	protected:
		/**
		 * Sensor identification string
		 */
		std::string mSensorID;

		/** 
		 * User-friendly sensor name
		 */
		std::string mSensorName;

		/**
		 * Time-zone string
		 */
		std::string mTimeZone;

		/**
         * Parameters of parking sensor
         */
		seevider::ParkingParams mParkingParams;

        /**
         * Parameters of lighting sensor
         */
        seevider::LightingParams mLightingParams;
	};
}
