#pragma once

#include <string>

#include <boost/property_tree/ptree.hpp>

namespace seevider {
	class SensorInfo {
	public:
		/**
		 * Sensor identification string
		 */
		const std::string &SensorID;

		/**
		 * Time-zone string, e.g., [+|-]##
		 */
		const std::string &TimeZone;

		/**
		 * Basic constructor
		 */
		SensorInfo();

		/**
		 * Basic destructor
		 */
		~SensorInfo();

	protected:
		/**
		 * Sensor identification string
		 */
		std::string mSensorID;

		/**
		 * Time-zone string
		 */
		std::string mTimeZone;
	};
}
