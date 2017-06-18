// File:	IOUtils.cpp
// Author:	Seongdo Kim
// Contact:	sdland85@gmail.com
//
// Copyright (c) 2017, Seongdo Kim <sdland85@gmail.com>
// All rights reserved.
// The copyright to the computer program(s) herein is
// the property of Seongdo Kim. The program(s) may be
// used and/or copied only with the written permission
// of Seongdo Kim or in accordance with the terms and
// conditions stipulated in the agreement/contract under
// which the program(s) have been supplied.
//
// Written by Seongdo Kim <sdland85@gmail.com>, June, 2017
//

#include "IOUtils.h"
#include "ParkingUpdateMessage.h"

#include <iostream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

#include <glog/logging.h>

namespace seevider {
	namespace utils {
		static const std::string base64_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		static inline bool is_base64(unsigned char c) {
			return (isalnum(c) || (c == '+') || (c == '/'));
		}

		bool loadOptions(const std::string filename) {
			return true;
		}

		void saveOptions(const std::string filename) {
			using boost::property_tree::ptree;

			ptree ptRoot;   // root structure
			ptree ptServer; // store server settings
			ptree ptSensor; // store sensor settings

			// Construct data array for server settings
			ptServer.put("IP", "000.000.000.000");
			ptServer.put("PORT", "00000");

			// Construct data array for sensor settings
			ptSensor.put("ROI", 0);

			write_json(filename, ptRoot);
		}

		void writeJSON(const std::string &filename, const boost::property_tree::ptree &ptree) {
			std::stringstream ss;
			std::ofstream fout(filename);

			write_json(ss, ptree);

			fout << ss.str();

			fout.close();
		}

		bool checkFolder(const std::string folder) {
			boost::filesystem::path p(folder);

			// If the storage folder does not exist, create it.
			if (!boost::filesystem::exists(p) || !boost::filesystem::is_directory(p)) {
				if (!boost::filesystem::create_directory(p)) {
					// TODO: If we failed to create a storage directory, we must fix it.
					// At least, we must notify the system what is the problem.
					std::cerr << "FATAL ERROR! failed to create the folder: " << p.string() << std::endl;
					return false;
				}
			}

			return true;
		}

		bool loadMessage(std::unique_ptr<seevider::IMessageData> &data, const std::string folder) {
			boost::filesystem::path p(folder);

			// If the storage folder does not exist, create it.
			if (!boost::filesystem::exists(p) || !boost::filesystem::is_directory(p)) {
				LOG(ERROR) << "The storage folder does not exist: " << p.string();
				return false;
			}

			// Pick the first file from the storage
			auto iter = boost::filesystem::directory_iterator(p);

			while (iter != boost::filesystem::directory_iterator()) {
				// Must decide which data we need to use to store the loaded data
				// Is this the best way to do?
				std::string ext = iter->path().extension().string();

				if (ext.compare(".png") == 0) {
					data = std::make_unique<seevider::ParkingUpdateMessage>();
					if (data->load(iter->path().string())) {
						// Remove the loaded file
						if (!boost::filesystem::remove(iter->path())) {
							// TODO: if we are not able to remove the loaded file, we should have problem. Fix it.
							LOG(ERROR) << "Failed to remove the file: " << iter->path().string();
							return false;
						}

						return true;
					}
				}
				else if (ext.compare(".avi") == 0) {
					LOG(WARNING) << "Not supported file type: " << iter->path().filename().string();
				}
				else {
					// TODO: if invalid file type is found, remove it.
					LOG(ERROR) << "Invalid file type: " << iter->path().filename().string();
				}

				iter++;
			}

			return false;
		}

		std::string base64_encode(const std::vector<uchar>& bytes_to_encode, unsigned int in_len) {
			std::string ret;
			int i = 0;
			int j = 0;
			unsigned char char_array_3[3];
			unsigned char char_array_4[4];
			auto iter = bytes_to_encode.cbegin();

			while (in_len--) {
				char_array_3[i++] = *(iter++);
				if (i == 3) {
					char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
					char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
					char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
					char_array_4[3] = char_array_3[2] & 0x3f;

					for (i = 0; (i <4); i++)
						ret += base64_chars[char_array_4[i]];
					i = 0;
				}
			}

			if (i)
			{
				for (j = i; j < 3; j++)
					char_array_3[j] = '\0';

				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (j = 0; (j < i + 1); j++)
					ret += base64_chars[char_array_4[j]];

				while ((i++ < 3))
					ret += '=';

			}

			return ret;

		}

		std::string base64_decode(std::string const& encoded_string) {
			int in_len = (int)encoded_string.size();
			int i = 0;
			int j = 0;
			int in_ = 0;
			unsigned char char_array_4[4], char_array_3[3];
			std::string ret;

			while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
				char_array_4[i++] = encoded_string[in_]; in_++;
				if (i == 4) {
					for (i = 0; i <4; i++)
						char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

					char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
					char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
					char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

					for (i = 0; (i < 3); i++)
						ret += char_array_3[i];
					i = 0;
				}
			}

			if (i) {
				for (j = i; j <4; j++)
					char_array_4[j] = 0;

				for (j = 0; j <4; j++)
					char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
			}

			return ret;
		}

		std::string base64_encode_image(const cv::Mat &image) {
			std::vector<uchar> buf;
			cv::imencode(".jpg", image, buf);

			return base64_encode(buf, (unsigned int)buf.size());
		}

		cv::Mat base64_decode_image(std::string const& s) {
			std::string buf = base64_decode(s);
			std::vector<uchar> dec_msg;

			for (int i = 0; i < buf.size(); i++) {
				dec_msg.push_back(buf[i]);
			}

			return cv::imdecode(dec_msg, CV_LOAD_IMAGE_COLOR);
		}
	}
}