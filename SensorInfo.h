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
		 * User-friendly sensor name
		 */
		const std::string &SensorName;

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
		 * User-friendly sensor name
		 */
		std::string mSensorName;

		/**
		 * Time-zone string
		 */
		std::string mTimeZone;
	};
}
