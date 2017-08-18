// File:	ParkingSpotManager.h
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

#include "ParkingSpot.h"

namespace seevider {
	class ParkingSpotManager
	{
	public:
		// Type definition of iterator
		typedef std::unordered_map<int, std::shared_ptr<ParkingSpot>>::iterator iterator;

		// Type definition of constant iterator
		typedef std::unordered_map<int, std::shared_ptr<ParkingSpot>>::const_iterator const_iterator;

	private:
		/**
		 * Hash array of parking spots
		 */
		std::unordered_map<int, std::shared_ptr<ParkingSpot>> mParkingSpots;

	public:
		ParkingSpotManager();
		~ParkingSpotManager();

		/**
		 * Add one parking spot. If a parkign spot with the same id is already exist, do not add and return false.
		 */
		bool add(int id, std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy);

		/**
		 * Update one parking spot with the given id. If the parking spot does not exist, return false.
		 */
		bool update(int id, std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy);

		/**
		 * Erase one parking spot with given id.
		 */
		bool erase(int id);

		/**
		 * Erase all the parking spots.
		 */
		void clear();

		/**
		 * Reset parking status of all the parking spots.
		 */
		void reset();

		/**
		 * Get the number of parking spots
		 */
		unsigned int size() const;

		/**
		 * Check if the list of parking spot is empty
		 */
		bool empty() const;
		
		/**
		 * Read parking spots from given json file
		 */
		bool readFromJSONFile(std::string filename);

		/**
		 * Update parking spots from given property tree.
		 * If parking spots of given id is exist, it will update the data of the parking spot.
		 * If parking spots of given id is not exist, it will add the new parking spot.
		 * If the ID of an existing parking spot is not in the given property tree, remove the parking spot.
		 */
		bool updateParkingSpots(boost::property_tree::ptree &root);

		/**
		 * Convert parking status to property tree
		 */
		boost::property_tree::ptree toPTree() const;

		/**
		 * Draw parking ROI and its status on given image
		 */
		cv::Mat drawParkingStatus(cv::Mat frame) const;

		/**
		 * The beginning iterator of parking spots
		 */
		iterator begin();

		/**
		 * The beginning constant iterator of parking spots
		 */
		const_iterator begin() const;

		/**
		 * The last iterator of parking spots
		 */
		iterator end();

		/**
		 * The last constant iterator of parking spots
		 */
		const_iterator end() const;

		/**
		 * Access parking spot of given id. Returned ParkingSpot can be modified.
		 */
		std::shared_ptr<ParkingSpot> operator[](const int id);

		/**
		 * Access parking spot of given id.
		 */
		const std::shared_ptr<ParkingSpot> operator[](const int id) const;

	private:
	};
}
