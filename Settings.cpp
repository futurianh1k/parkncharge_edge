#include "Settings.h"

#include <iostream>
#include <boost/property_tree/ini_parser.hpp>

#include "Utils.h"

namespace seevider {
	Settings::Settings(std::string filename) : mSettingsFilename(filename) {
		loadSettings(filename);
	}

	Settings::~Settings() {
		writeSettings(mSettingsFilename);
	}

	bool Settings::loadSettings(const std::string filename) {
		using std::cout;
		using std::endl;

		std::string value;
		boost::property_tree::ptree ptree;

		try {
			boost::property_tree::read_ini(filename, ptree);
		}
		catch (const boost::property_tree::ptree_error &e) {
			cout << e.what() << endl;
			return false;
		}

		if (ptree.empty()) {
			cout << "The option file is empty: " << filename << endl;
			return false;
		}

		value = ptree.get<std::string>("SensorID", "");
		if (!value.empty()) {
			mSensorID = value;
		}
		else {
			cout << "Failed to read option \'SensorID\'" << endl;
		}

		// Read the option 'ServerData'
		ServerDataFilename = ptree.get<std::string>("ServerData", "");
		if (ServerDataFilename.empty()) {
			cout << "Failed to read option \'ServerData\'" << endl;
		}

		// Read the option 'ServerAddress'
		value = ptree.get<std::string>("MotionDetection", "");
		if (value.empty()) {
			cout << "Failed to read option \'MotionDetection\'" << endl;
			MotionDetectionEnabled = false;
		}
		else {
			MotionDetectionEnabled = to_bool(value);
		}

		// Read the Time-Zone parameter
		mTimeZone = ptree.get<std::string>("TimeZone", "");
		if (mTimeZone.empty()) {
			cout << "Failed to read option \'TimeZone\'" << endl;
			mTimeZone = "+00";
		}

		// Read the detection engine type
		value = ptree.get<std::string>("DetectorType", "");
		if (!value.empty()) {
			if (value.compare("cascade") == 0) {
				Type = CLASSIFIER_CASCADE;

			}
			else if (value.compare("cnn") == 0) {
				Type = CLASSIFIER_CNN;
			}
			else {
				cout << "Critical error! Undefined classifier type: " << value << endl;
				return false;
			}

			value = ptree.get<std::string>("TrainedFilename", "");
			if (value.empty()) {
				cout << "Critical error! Failed to read option \'TrainedFilename\'" << endl;
			}
			else {
				TrainedFilename = value;
			}
		}
		else {
			cout << "Critical error! Failed to read option \'DetectorType\'" << endl;
			return false;
		}

		return true;
	}

	bool Settings::writeSettings(const std::string filename) {
		boost::property_tree::ptree ptree;
		std::ofstream fout(filename);
		std::stringstream ss;

		// Check if file was opened without any error
		if (!fout.is_open()) {
			std::cerr << stderr << std::endl;
			return false;
		}

		// Write the option 'ServerAddress'
		ptree.put<std::string>("ServerData", ServerDataFilename);

		// Write the option 'MotionDetection'
		ptree.put<std::string>("MotionDetection", to_string(MotionDetectionEnabled));

		// Write the option 'TimeZone'
		ptree.put<std::string>("TimeZone", TimeZone);

		// Write the option 'DetectorType'
		ptree.put<std::string>("DetectorType", CLASSIFIER_TYPE_STRING[Type]);

		// Write the option 'DetectorType'
		ptree.put<std::string>("TrainedFilename", TrainedFilename);

		// Write INI file
		write_ini(ss, ptree);
		fout << ss.str();
		fout.close();

		return true;
	}
}
