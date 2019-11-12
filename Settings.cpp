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
		std::cout << std::endl << "----------<<< Settings.cpp in  >>>----------" << std::endl << std::endl;
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

		//-------------------------------------
		// Read sensor information
		//-------------------------------------

		// Read the option 'SensorID'
		value = ptree.get<std::string>("Sensor.SensorID", "");
		if (!value.empty()) {
			mSensorID = value;
		}
		else {
			LOG(ERROR) << "Failed to read option \'SensorID\'";
			// TODO: set sensor ID to default ID, or connect server to check its ID
		}

		// Read the option 'ServerData'
		ServerDataFilename = ptree.get<std::string>("Sensor.ServerData", "");
		if (ServerDataFilename.empty()) {
			LOG(FATAL) << "Failed to read option \'ServerData\'";
			return false;
		}

		// Read the Time-Zone parameter
		mTimeZone = ptree.get<std::string>("Sensor.TimeZone", "");
		if (mTimeZone.empty()) {
			LOG(ERROR) << "Failed to read option \'TimeZone\'";
			mTimeZone = "+00";
		}

		// Read the option 'Sensitivity'
		mParkingParams.sensitivity = ptree.get<int>("Sensor.Sensitivity");
		if (mParkingParams.sensitivity <= 0) {
			LOG(ERROR) << "Failed to read option \'Sensitivity\'";
			mParkingParams.sensitivity = 3;
		}

		// Read the option 'EnterCount'
		mParkingParams.enterCount = ptree.get<int>("Sensor.EnterCount");
		if (mParkingParams.enterCount <= 0) {
			LOG(ERROR) << "Failed to read option \'EnterCount\'";
			mParkingParams.enterCount = 6;
		}

		// Read the option 'ExitCount'
		mParkingParams.exitCount = ptree.get<int>("Sensor.ExitCount");
		if (mParkingParams.exitCount <= 0) {
			LOG(ERROR) << "Failed to read option \'ExitCount\'";
			mParkingParams.exitCount = 3;
		}

		//-------------------------------------
		// Read camera settings
		//-------------------------------------

		// Read the option 'FrameWidth'
		mFrameWidth = ptree.get<int>("Camera.FrameWidth", -1);
		if (mFrameWidth <= 0) {
			LOG(ERROR) << "Failed to read option \'FrameWidth\'";
		}

		// Read the option 'FrameHeight'
		mFrameHeight = ptree.get<int>("Camera.FrameHeight", -1);
		if (mFrameHeight <= 0) {
			LOG(ERROR) << "Failed to read option \'FrameHeight\'";
		}

		// Read the option 'FourCC'
		mFourCC = ptree.get<std::string>("Camera.FourCC", "");
		if (mFourCC.empty()) {
			LOG(ERROR) << "Failed to read option \'FourCC\'";
		}


		//-------------------------------------
                // Read File settings
                //-------------------------------------

                // Read the option 'ParkingSpot ROI JSON Filename'
                ParkingSpotROIJsonFilename = ptree.get<std::string>("File.SpotFilename", "");
                if (ParkingSpotROIJsonFilename.empty()) {
                        LOG(ERROR) << "Failed to read option \'SpotFilename\'.";
                }
                //SYSTEM_FILE_PARKINGSPOTS = ParkingSpotROIJsonFilename;


                // Read the option 'VideoFilename'
                // 카메라 직접 연결시 카메라 usb 포트 번호 입력
                // 녹화된 영상 띄울땐 영상 파일 이름 입력
                VideoFilename = ptree.get<std::string>("File.VideoFilename", "");
                if (VideoFilename.empty()) {
                        LOG(ERROR) << "Failed to read option \'VideoFilename\'.";
                }

		return true;
	}


    void Settings::updateParkingParams(boost::property_tree::ptree &root){
		// Update the value 'sensitivity'
		mParkingParams.sensitivity = root.get<int>("sensitivity");

		// Update the value 'enterCount'
		mParkingParams.enterCount = root.get<int>("enterCount");

		// Update the value 'exitCount'
		mParkingParams.exitCount = root.get<int>("exitCount");

		// Write INI file
		writeSettings();
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

        // Write the option 'SensorID'
		ptree.put<std::string>("Sensor.SensorID", SensorID);

		// Write the option 'ServerAddress'
		ptree.put<std::string>("Sensor.ServerData", ServerDataFilename);

		// Write the option 'TimeZone'
		ptree.put<std::string>("Sensor.TimeZone", TimeZone);

		// Write the option 'Sensitivity'
		ptree.put<int>("Sensor.Sensitivity", ParkingParams.sensitivity);

		// Write the option 'EnterCount'
		ptree.put<int>("Sensor.EnterCount", ParkingParams.enterCount);

		// Write the option 'ExitCount'
		ptree.put<int>("Sensor.ExitCount", ParkingParams.exitCount);

		// Write the option 'FrameWidth'
		ptree.put<int>("Camera.FrameWidth", mFrameWidth);

		// Write the option 'FrameHeight'
		ptree.put<int>("Camera.FrameHeight", mFrameHeight);

		// Write the option 'FourCC'
		ptree.put<std::string>("Camera.FourCC", FourCC);

		// Write the option 'SpotFilename'
                ptree.put<std::string>("File.SpotFilename", ParkingSpotROIJsonFilename);

                // Write the option 'VideoFilename'
                ptree.put<std::string>("File.VideoFilename", VideoFilename);

		// Write INI file
		write_ini(ss, ptree);
		fout << ss.str();
		fout.close();

		return true;
	}
}
