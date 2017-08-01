// File:	Settings.h
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

#include "types.h"
#include "SensorInfo.h"
#include "CameraInfo.h"

namespace seevider {
	class Settings : public SensorInfo, public CameraInfo {
	public:
		/**
		 * Trained filename for the detection
		 */
		std::string TrainedFilename;

		/**
		 * LPR region code
		 */
		std::string LPRRegionCode;

		/**
		 * LPR settings filename
		 */
		std::string LPRSettingsFilename;

		/**
		 * Server settings
		 */
		std::string ServerDataFilename;

		/**
		 * Enability of motion detection
		 */
		bool MotionDetectionEnabled;

		/**
		 * Type of detection engine
		 */
		CLASSIFIER_TYPE Type;

		/**
		 * Default constructor.
		 */
		Settings(std::string filename);

		/**
		 * Default destructor
		 */
		~Settings();

		/**
		 * Load current options to given filename
		 */
		bool loadSettings();

	private:
		/**
		 * Files to load and store the setting data
		 */
		std::string mSettingsFilename;

		/**
		 * Write current options to given filename
		 */
		bool writeSettings();
	};
}