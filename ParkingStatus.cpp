#include "ParkingStatus.h"

#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace seevider;

ParkingStatus::ParkingStatus() {

}

ParkingStatus::ParkingStatus(const int update_code, const std::string spotID,
    const cv::Mat &frame, const boost::posix_time::ptime &eventTime) :
    mUpdateCode(update_code), mSpotID(spotID),
    mFrame(frame.clone()), mEventTime(eventTime) {
    // do nothing
}

ParkingStatus::~ParkingStatus() {

}

std::string ParkingStatus::toString() const {
    std::stringstream sstr;

    sstr << mSpotID << " at " << to_simple_string(mEventTime);

    return sstr.str();
}

std::string ParkingStatus::toJSONObject() const {
    std::ostringstream output;

    return output.str();
}