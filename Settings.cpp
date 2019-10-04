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

		// Read the option 'NoiseFilterSize'
		mLightingParams.noiseFilterSize = ptree.get<int>("Sensor.NoiseFilterSize");
		if (mLightingParams.noiseFilterSize <= 0) {
			LOG(FATAL) << "Failed to read option \'NoiseFilterSize\'";
			return false;
		}

		// Read the option 'MinMotionArea'
		mLightingParams.minMotionArea = ptree.get<int>("Sensor.MinMotionArea");
		if (mLightingParams.minMotionArea <= 0) {
			LOG(FATAL) << "Failed to read option \'MinMotionArea\'";
			return false;
		}

		// Read the option 'MaxMotionArea'
		mLightingParams.maxMotionArea = ptree.get<int>("Sensor.MaxMotionArea");
		if (mLightingParams.maxMotionArea <= 0) {
			LOG(FATAL) << "Failed to read option \'MaxMotionArea\'";
			return false;
		}

		// Read the option 'LightDelayTime'
		mLightingParams.lightDelayTime = ptree.get<int>("Sensor.LightDelayTime");
		if (mLightingParams.lightDelayTime <= 0) {
			LOG(FATAL) << "Failed to read option \'LightDelayTime\'";
			return false;
		}

		// Read the option 'LightDimdownTime'
		mLightingParams.lightDimdownTime = ptree.get<int>("Sensor.LightDimdownTime");
		if (mLightingParams.lightDimdownTime <= 0) {
			LOG(FATAL) << "Failed to read option \'LightDimdownTime\'";
			return false;
		}

		// Read the option 'LightMaxLevel'
		mLightingParams.lightMaxLevel = ptree.get<int>("Sensor.LightMaxLevel");
		if (mLightingParams.lightMaxLevel <= 0) {
			LOG(FATAL) << "Failed to read option \'LightMaxLevel\'";
			return false;
		}

		// Read the option 'LightMinLevel'
		mLightingParams.lightMinLevel = ptree.get<int>("Sensor.LightMinLevel");
		if (mLightingParams.lightMinLevel <= 0) {
			LOG(FATAL) << "Failed to read option \'LightMinLevel\'";
			return false;
		}

		//-------------------------------------
		// Read algorithm settings
		//-------------------------------------

		// Read the option 'MotionDetection'
		value = ptree.get<std::string>("Algorithm.MotionDetection", "");
		if (value.empty()) {
			LOG(ERROR) << "Failed to read option \'MotionDetection\'";
			MotionDetectionEnabled = false;
		}
		else {
			MotionDetectionEnabled = to_bool(value);
		}

		// Read the detection engine type
		value = ptree.get<std::string>("Algorithm.DetectorType", "");
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

			value = ptree.get<std::string>("Algorithm.TrainedFilename", "");
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

		// Read the option 'LPTrainedFilename

		value = ptree.get<std::string>("Algorithm.LPTrainedFilename", "");
		if (value.empty()) {
			LOG(FATAL) << "Failed to read option \'LPTrainedFilename\'";
		}
		else {
			LPTrainedFilename = value;
		}

		// Read the option 'LPRRegionCode'
		LPRRegionCode = ptree.get<std::string>("Algorithm.LPRRegionCode", "");
		if (LPRRegionCode.empty()) {
			LOG(ERROR) << "Failed to read option \'LPRRegionCode\'. Set to the default region \'us\'";
			LPRRegionCode = "us";
		}

		// Read the option 'LPRSettingsFilename'
		LPRSettingsFilename = ptree.get<std::string>("Algorithm.LPRSettingsFilename", "");
		if (LPRSettingsFilename.empty()) {
			LOG(FATAL) << "Failed to read option \'LPRSettingsFilename\'. Turn off the LPR engine";
			LPRSettingsFilename = "";
		}
				// Read the option 'CfgFile'
		CfgFile = ptree.get<std::string>("Algorithm.CfgFile", "");
		if (CfgFile.empty()) {
			LOG(FATAL) << "Failed to read option \'CfgFile\'";
		       	CfgFile = "";
	       	}

		// Read the option 'WeightFile'
		WeightFile = ptree.get<std::string>("Algorithm.WeightFile", "");
		if (WeightFile.empty()) {
			LOG(FATAL) << "Failed to read option \'WeightFile\'";
		       	WeightFile = "";
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


    void Settings::updateLightingParams(boost::property_tree::ptree &root){
		// Update the value 'noiseFilterSize'
        mLightingParams.noiseFilterSize = root.get<int>("noiseFilterSize");

		// Update the value 'minMotionArea'
        mLightingParams.minMotionArea = root.get<int>("minMotionArea");

		// Update the value 'maxMotionArea'
        mLightingParams.maxMotionArea = root.get<int>("maxMotionArea");

		// Update the value 'lightDelayTime'
        mLightingParams.lightDelayTime = root.get<int>("lightDelayTime");

		// Update the value 'lightDimdownTime'
        mLightingParams.lightDimdownTime = root.get<int>("lightDimdownTime");

		// Update the value 'lightMinLevel'
        mLightingParams.lightMinLevel = root.get<int>("lightMinLevel");

		// Update the value 'lightMaxLevel'
        mLightingParams.lightMaxLevel = root.get<int>("lightMaxLevel");

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

		// Write the option 'NoiseFilterSize'
		ptree.put<int>("Sensor.NoiseFilterSize", LightingParams.noiseFilterSize);

		// Write the option 'MinMotionArea'
		ptree.put<int>("Sensor.MinMotionArea", LightingParams.minMotionArea);

		// Write the option 'MaxMotionArea'
		ptree.put<int>("Sensor.MaxMotionArea", LightingParams.maxMotionArea);

		// Write the option 'LightDelayTime'
		ptree.put<int>("Sensor.LightDelayTime", LightingParams.lightDelayTime);

		// Write the option 'LightDimdownTime'
		ptree.put<int>("Sensor.LightDimdownTime", LightingParams.lightDimdownTime);

		// Write the option 'LightMaxLevel'
		ptree.put<int>("Sensor.LightMaxLevel", LightingParams.lightMaxLevel);

		// Write the option 'LightMinLevel'
		ptree.put<int>("Sensor.LightMinLevel", LightingParams.lightMinLevel);

		// Write the option 'MotionDetection'
		ptree.put<std::string>("Algorithm.MotionDetection", to_string(MotionDetectionEnabled));

		// Write the option 'DetectorType'
		ptree.put<std::string>("Algorithm.DetectorType", CLASSIFIER_TYPE_STRING[Type]);

		// Write the option 'TrainedFilename'
		ptree.put<std::string>("Algorithm.TrainedFilename", TrainedFilename);

		// Write the option 'LPTrainedFilename'
		ptree.put<std::string>("Algorithm.LPTrainedFilename", LPTrainedFilename);

		// Write the option 'LPRRegionCode'
		ptree.put<std::string>("Algorithm.LPRRegionCode", LPRRegionCode);

		// Write the option 'LPRSettingsFilename'
		ptree.put<std::string>("Algorithm.LPRSettingsFilename", LPRSettingsFilename); 

		// Write the option 'CfgFile'
		ptree.put<std::string>("Algorithm.CfgFile", CfgFile);

		// Write the option 'WeightFile'
		ptree.put<std::string>("Algorithm.WeightFile", WeightFile);

		// Write the option 'FrameWidth'
		ptree.put<int>("Camera.FrameWidth", mFrameWidth);

		// Write the option 'FrameHeight'
		ptree.put<int>("Camera.FrameHeight", mFrameHeight);

		// Write the option 'FourCC'
		ptree.put<std::string>("Camera.FourCC", FourCC);

		// Write INI file
		write_ini(ss, ptree);
		fout << ss.str();
		fout.close();

		return true;
	}
}
