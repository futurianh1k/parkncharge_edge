// File:	ServerSyncMessage.cpp
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

#include "types.h"
#include "ServerSyncMessage.h"

#include <iostream>
namespace seevider {
	ServerSyncMessage::ServerSyncMessage(const cv::Size imageSize,
		const boost::posix_time::ptime eventTime,
		const boost::property_tree::ptree& parkingSpots) : IMessageData(eventTime) {
		std::cout << std::endl << "----------<<< ServerSyncMessage.cpp in  >>>----------" << std::endl << std::endl;
		mImageSize = imageSize;
		mJSONParkingSpots = parkingSpots;
	}


	ServerSyncMessage::~ServerSyncMessage() {
	}

	std::string ServerSyncMessage::toString() const {
		// TODO: implement
		return std::string();
	}

	boost::property_tree::ptree ServerSyncMessage::toPTree() const {
		boost::property_tree::ptree info;
		std::string eventTime = boost::posix_time::to_iso_string(mEventTime);

		eventTime.erase(8, 1);

		info.put<int>("httpRequest", HTTP_REQ_SYNC_GENERAL);
		//info.put<std::string>("timeStamp", boost::posix_time::to_iso_string(mEventTime));
		info.put<int>("messageType", 0);
		info.put<std::string>("timeStamp", eventTime);
		info.put<int>("pictureWidth", mImageSize.width);
		info.put<int>("pictureHeight", mImageSize.height);
		info.add_child("ROI", mJSONParkingSpots);
		return info;
	}

	bool ServerSyncMessage::save(std::string folder) const {
		// TODO: implement
		return true;
	}

	bool ServerSyncMessage::load(const std::string filename) {
		// TODO: implement
		return true;
	}
}
