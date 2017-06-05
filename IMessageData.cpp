#include "IMessageData.h"

namespace seevider {
	IMessageData::IMessageData() {

	}

	IMessageData::IMessageData(boost::posix_time::ptime eventTime) : mEventTime(eventTime) {
	}
}
