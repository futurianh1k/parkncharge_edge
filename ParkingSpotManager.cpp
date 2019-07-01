// File:	ParkingSpotManager.cpp
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

#include "ParkingSpotManager.h"

#include <unordered_set>

#include <glog/logging.h>

#include <boost/property_tree/json_parser.hpp>

namespace seevider {
	ParkingSpotManager::ParkingSpotManager() {
		if (!readFromJSONFile(SYSTEM_FOLDER_CORE + SYSTEM_FILE_PARKINGSPOTS)) {
			LOG(WARNING) << "Cannot initialize parking spots from system file: " << SYSTEM_FILE_PARKINGSPOTS;
		}
	}

	ParkingSpotManager::~ParkingSpotManager() {
		boost::property_tree::write_json(SYSTEM_FOLDER_CORE + SYSTEM_FILE_PARKINGSPOTS, toPTree());
	}

	bool ParkingSpotManager::add(int id, std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy) {
		if (mParkingSpots.find(id) != mParkingSpots.cend()) {
			return false;
		}

		mParkingSpots[id] = std::make_shared<ParkingSpot>(id, spotName, timeLimit, roi, policy);

		return true;
	}

	bool ParkingSpotManager::update(int id, std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy) {
		iterator spot = mParkingSpots.find(id);
		if (spot == mParkingSpots.end()) {
			return false;
		}

		return spot->second->update(spotName, timeLimit, roi, policy);
	}

	bool ParkingSpotManager::erase(int id) {
		iterator spot = mParkingSpots.find(id);
		if (spot == mParkingSpots.end()) {
			return false;
		}

		spot->second->reset();
		mParkingSpots.erase(id);

		return true;
	}

	void ParkingSpotManager::clear() {
		reset();
		mParkingSpots.clear();
	}

	void ParkingSpotManager::reset() {
		for (auto &spot : mParkingSpots) {
			spot.second->reset();
		}
	}

	unsigned int ParkingSpotManager::size() const {
		return (unsigned int)mParkingSpots.size();
	}

	bool ParkingSpotManager::empty() const {
		return mParkingSpots.empty();
	}

	bool ParkingSpotManager::readFromJSONFile(std::string filename) {
		boost::property_tree::ptree ptree;

		try {
			boost::property_tree::read_json(filename, ptree);
		}
		catch (const boost::property_tree::ptree_error &e) {
			LOG(ERROR) << e.what();
			return false;
		}

		for (auto &elem : ptree) {
			int id, timeLimit;
			std::string spotName;
			PARKING_SPOT_POLICY policy;
			cv::Rect roi;

			try {
				id = elem.second.get<int>("parkingSpotId");
				spotName = elem.second.get<std::string>("parkingSpotName");
				roi.x = elem.second.get<int>("roiCoordX");
				roi.y = elem.second.get<int>("roiCoordY");
				roi.width = elem.second.get<int>("roiWidth");
				roi.height = elem.second.get<int>("roiHeight");
				policy = (PARKING_SPOT_POLICY)elem.second.get<int>("policyId");
				timeLimit = elem.second.get<int>("timeLimit");

				add(id, spotName, timeLimit, roi, policy);
			}
			catch (const boost::property_tree::ptree_error &e) {
				LOG(ERROR) << e.what();
			}
		}

		return true;
	}

	bool ParkingSpotManager::updateParkingSpots(boost::property_tree::ptree &root) {
		std::unordered_set<int> updated;

		for (auto elem : root) {
			int id, timeLimit;
			std::string spotName;
			PARKING_SPOT_POLICY policy;
			cv::Rect roi;

			try {
				id = elem.second.get<int>("parkingSpotId");
				spotName = elem.second.get<std::string>("parkingSpotName");
				roi.x = elem.second.get<int>("roiCoordX");
				roi.y = elem.second.get<int>("roiCoordY");
				roi.width = elem.second.get<int>("roiWidth");
				roi.height = elem.second.get<int>("roiHeight");
				policy = (PARKING_SPOT_POLICY)elem.second.get<int>("policyId");
				timeLimit = elem.second.get<int>("timeLimit");

				std::cout << id << ": " << spotName << ", " << roi << ", " << policy << ", " << timeLimit << std::endl;
			}
			catch (const boost::property_tree::ptree_error &e) {
				LOG(ERROR) << e.what();
			}

			auto ptr = mParkingSpots.find(id);
			if (ptr == mParkingSpots.end()) {
				add(id, spotName, timeLimit, roi, policy);
			}
			else {
				update(id, spotName, timeLimit, roi, policy);
			}

			updated.insert(id);
		}

		// Remove parking spots that are not in 'root'
		for (auto iter = mParkingSpots.begin(); iter != mParkingSpots.end();) {
			if (updated.find(iter->first) != updated.end()) {
				iter++;
			}
			else {
				iter = mParkingSpots.erase(iter);
			}
		}

		// Write json file
		boost::property_tree::write_json(SYSTEM_FOLDER_CORE + SYSTEM_FILE_PARKINGSPOTS, toPTree());

		return true;
	}

	boost::property_tree::ptree ParkingSpotManager::toPTree() const {
		boost::property_tree::ptree spotArray;
		
		for (auto elem : mParkingSpots) {
			std::shared_ptr<ParkingSpot> &spot = elem.second;
			boost::property_tree::ptree result;

			result.put("parkingSpotId", std::to_string(spot->ID));
			result.put("parkingSpotName", spot->SpotName);
			result.put("roiCoordX", spot->ROI.x);
			result.put("roiCoordY", spot->ROI.y);
			result.put("roiWidth", spot->ROI.width);
			result.put("roiHeight", spot->ROI.height);
			result.put("policyId", (int)spot->ParkingPolicy);
			result.put("timeLimit", spot->TimeLimit);

			spotArray.push_back(std::make_pair("", result));
		}

		return spotArray;
	}

	cv::Mat ParkingSpotManager::drawParkingStatus(cv::Mat frame) const {
		cv::Mat drawn = frame.clone();

		for (auto parkingSpot : mParkingSpots) {
			cv::Scalar color;
			if (parkingSpot.second->isOverstayed()) {
				color = CV_RGB(0, 0, 255);
                               
			}
                        else if(parkingSpot.second->isOccupied()){
                                color = CV_RGB(255, 0, 0);
                              
                              
                        }
			else {
				color = CV_RGB(0, 255, 0);
                          
			}

			cv::rectangle(drawn, parkingSpot.second->ROI, color, 2);
		}

		return drawn;
	}

	ParkingSpotManager::iterator ParkingSpotManager::begin() {
		return mParkingSpots.begin();
	}

	ParkingSpotManager::const_iterator ParkingSpotManager::begin() const {
		return mParkingSpots.cbegin();
	}

	ParkingSpotManager::iterator ParkingSpotManager::end() {
		return mParkingSpots.end();
	}

	ParkingSpotManager::const_iterator ParkingSpotManager::end() const {
		return mParkingSpots.cend();
	}

	std::shared_ptr<ParkingSpot> ParkingSpotManager::operator[](const int id) {
		return mParkingSpots[id];
	}

	const std::shared_ptr<ParkingSpot> ParkingSpotManager::operator[](const int id) const {
		const_iterator spot = mParkingSpots.find(id);

		if (spot != mParkingSpots.cend()) {
			return spot->second;
		}
		else {
			return nullptr;
		}
	}
}
