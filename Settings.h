#pragma once

#include <string>

#include "types.h"
#include "SensorInfo.h"

namespace seevider {
	class Settings : public SensorInfo {
	public:
		/**
		 * Trained filename for the detection
		 */
		std::string TrainedFilename;

		/**
		 * Server settings
		 */
		std::string ServerDataFilename;

		/**
		 * Enability of motion detection
		 */
		bool MotionDetectionEnabled;

		/**
		 * Type of detection engine
		 */
		CLASSIFIER_TYPE Type;

		/**
		 * Default constructor.
		 */
		Settings(std::string filename);

		/**
		 * Default destructor
		 */
		~Settings();

		/**
		 * Load current options to given filename
		 */
		bool loadSettings();

	private:
		/**
		 * Files to load and store the setting data
		 */
		std::string mSettingsFilename;

		/**
		 * Write current options to given filename
		 */
		bool writeSettings();
	};
}