#include "ParkingUpdateMessage.h"
#include "IOUtils.h"

#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace seevider;

ParkingUpdateMessage::ParkingUpdateMessage() {
};

ParkingUpdateMessage::ParkingUpdateMessage(const int request, const int spotID,
    const cv::Mat &frame, const boost::posix_time::ptime &eventTime) :
	IMessageData(eventTime), mRequestCode(request), mSpotID(spotID),
	mFrame(frame.clone()) {
    // do nothing
}

ParkingUpdateMessage::~ParkingUpdateMessage() {

}

std::string ParkingUpdateMessage::toString() const {
    std::stringstream sstr;

	sstr << mSpotID << " at " << to_simple_string(mEventTime) << " (" << mRequestCode << ")";

    return sstr.str();
}

boost::property_tree::ptree ParkingUpdateMessage::toPTree() const {
	boost::property_tree::ptree info;
	
	info.put<int>("messageType", 1);
	info.put<int>("httpRequest", mRequestCode);
	info.put<int>("parkingSpotId", mSpotID);
	info.put<std::string>("timeStamp", boost::posix_time::to_iso_string(mEventTime));
	info.put<std::string>("currentPicture", utils::base64_encode_image(mFrame));

	return info;
}

bool ParkingUpdateMessage::save(std::string folder) const {
	cv::String filename = folder + constructFilename();

	if (!utils::checkFolder(folder)) {
		// TODO: Folder check failed. Figure out what is the problem, e.g., maybe we have lack
		// of available space If possible, Resolve the problem and try to check it again.
		return false;
	}

	if (!cv::imwrite(filename, mFrame)) {
		// TODO: Save failed. Figure out what is the problem, e.g., maybe we have lack
		// of available space If possible, fix the problem and try to save it again.
		return false;
	}

	return true;
}

bool ParkingUpdateMessage::load(const std::string filename) {
	boost::filesystem::path p(filename);
	std::vector<std::string> substrs;
	boost::split(substrs, p.stem().string(), boost::is_any_of("_"));

	if (substrs.size() < 3) {
		std::cout << "ERROR! invalid filename." << std::endl;
		return false;	// failed to parse filename
	}

	// Set the parsed data
	mFrame = cv::imread(filename);
	if (mFrame.empty()) {
		// Not able to read image
		std::cout << "ERROR! failed to read image." << std::endl;
		return false;
	}

	mEventTime = boost::posix_time::from_iso_string(substrs[0]);
	mRequestCode = std::stoi(substrs[1]);
	if (substrs.size() == 3) {
		mSpotID = stoi(substrs[2]);
	}
	else if (substrs.size() == 4) {
		mSpotID = stoi(substrs[2] + substrs[3]);
	}
	else {
		mSpotID = stoi(boost::algorithm::join(std::vector<std::string>(substrs.begin() + 2, substrs.end()), ""));
	}

	return true;
}

cv::String ParkingUpdateMessage::constructFilename() const {
	//std::ostringstream sstr;
	
	//boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%Y%m%d_%H%M%S");
	//sstr.imbue(std::locale(std::cout.getloc(), facet));
	//sstr << mEventTime << "_" << mSpotID << ".png";

	//return sstr.str();
	return boost::posix_time::to_iso_string(mEventTime) + "_" + std::to_string(mRequestCode) + "_" + std::to_string(mSpotID) + ".png";
}
