#include "SensorInfo.h"

namespace seevider {
	SensorInfo::SensorInfo() :
		SensorID(mSensorID), SensorName(mSensorName), TimeZone(mTimeZone) {
		//mSensorID = sensorID;
	};

	SensorInfo::~SensorInfo() {
	}
}
