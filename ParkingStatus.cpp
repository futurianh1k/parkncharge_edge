#include "ParkingStatus.h"

#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace seevider;

ParkingStatus::ParkingStatus(const int update_code, const int spotID,
    const cv::Mat &frame, boost::posix_time::ptime eventTime) :
    mUpdateCode(update_code), mSpotID(spotID),
    mFrame(frame.clone()), mTime(eventTime) {
    // do nothing
}

ParkingStatus::~ParkingStatus() {

}

std::string ParkingStatus::toString() const {
    // TODO: implement it
    return std::string();
}

std::string ParkingStatus::toJSONObject() const {
    std::ostringstream output;

    return output.str();
}