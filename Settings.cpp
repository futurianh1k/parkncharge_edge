// File:	Settings.cpp
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

#include "Settings.h"

#include "Utils.h"

#include <iostream>
#include <boost/property_tree/ini_parser.hpp>

#include <glog/logging.h>

namespace seevider {
	Settings::Settings(std::string filename) : mSettingsFilename(filename) {
		loadSettings();
	}

	Settings::~Settings() {
		writeSettings();
	}

	bool Settings::loadSettings() {
		using std::cout;
		using std::endl;

		std::string value;
		boost::property_tree::ptree ptree;

		try {
			boost::property_tree::read_ini(mSettingsFilename, ptree);
		}
		catch (const boost::property_tree::ptree_error &e) {
			LOG(FATAL) << e.what();
			return false;
		}

		if (ptree.empty()) {
			LOG(FATAL) << "The option file is empty: " << mSettingsFilename;
			return false;
		}

		// Read the option 'SensorID'
		value = ptree.get<std::string>("SensorID", "");
		if (!value.empty()) {
			mSensorID = value;
		}
		else {
			LOG(ERROR) << "Failed to read option \'SensorID\'";
			// TODO: set sensor ID to default ID, or connect server to check its ID
		}

		// Read the option 'ServerData'
		ServerDataFilename = ptree.get<std::string>("ServerData", "");
		if (ServerDataFilename.empty()) {
			LOG(FATAL) << "Failed to read option \'ServerData\'";
			return false;
		}

		// Read the option 'MotionDetection'
		value = ptree.get<std::string>("MotionDetection", "");
		if (value.empty()) {
			LOG(ERROR) << "Failed to read option \'MotionDetection\'";
			MotionDetectionEnabled = false;
		}
		else {
			MotionDetectionEnabled = to_bool(value);
		}

		// Read the Time-Zone parameter
		mTimeZone = ptree.get<std::string>("TimeZone", "");
		if (mTimeZone.empty()) {
			LOG(ERROR) << "Failed to read option \'TimeZone\'";
			mTimeZone = "+00";
		}

		// Read the detection engine type
		value = ptree.get<std::string>("DetectorType", "");
		if (!value.empty()) {
			if (value.compare("cascade") == 0) {
				Type = CLASSIFIER_CASCADE;

			}
			else if (value.compare("cnn") == 0) {
				Type = CLASSIFIER_CNN;
			}
			else {
				LOG(FATAL) << "Undefined classifier type: " << value;
				return false;
			}

			value = ptree.get<std::string>("TrainedFilename", "");
			if (value.empty()) {
				LOG(FATAL) << "Failed to read option \'TrainedFilename\'";
			}
			else {
				TrainedFilename = value;
			}
		}
		else {
			LOG(FATAL) << "Critical error! Failed to read option \'DetectorType\'";
			return false;
		}

		return true;
	}

	bool Settings::writeSettings() {
		boost::property_tree::ptree ptree;
		std::ofstream fout(mSettingsFilename);
		std::stringstream ss;

		// Check if file was opened without any error
		if (!fout.is_open()) {
			LOG(ERROR) << stderr;
			return false;
		}

		// Write the option 'ServerAddress'
		ptree.put<std::string>("SensorID", mSensorID);

		// Write the option 'ServerAddress'
		ptree.put<std::string>("ServerData", ServerDataFilename);

		// Write the option 'MotionDetection'
		ptree.put<std::string>("MotionDetection", to_string(MotionDetectionEnabled));

		// Write the option 'TimeZone'
		ptree.put<std::string>("TimeZone", TimeZone);

		// Write the option 'DetectorType'
		ptree.put<std::string>("DetectorType", CLASSIFIER_TYPE_STRING[Type]);

		// Write the option 'DetectorType'
		ptree.put<std::string>("TrainedFilename", TrainedFilename);

		// Write INI file
		write_ini(ss, ptree);
		fout << ss.str();
		fout.close();

		return true;
	}
}
