#include "types.h"
#include "ServerSyncMessage.h"

namespace seevider {
	ServerSyncMessage::ServerSyncMessage(const cv::Size imageSize,
		const boost::posix_time::ptime eventTime,
		const boost::property_tree::ptree& parkingSpots) : IMessageData(eventTime) {
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
