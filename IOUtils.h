// File:	IOUtils.h
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

#pragma once

#include "IMessageData.h"

#include <string>
#include <opencv2/opencv.hpp>

namespace seevider {
    namespace utils {
        /**
         * Load application options from given file
         */
        bool loadOptions(const std::string filename);

        /**
         * Save application options to given file
         */
        void saveOptions(const std::string filename);

		/**
		 * Save given json contents to given file
		 */
		void writeJSON(const std::string &filename, const boost::property_tree::ptree &ptree);

		/**
		 * Check if the folder exists. If the folder does not exist, create it.
		 * Returns true if success.
		 */
		bool checkFolder(const std::string folder);

		/**
		 * Load a message from given folder, which has the highest priority.
		 */
		bool loadMessage(std::unique_ptr<seevider::IMessageData> &data, const std::string folder);

		/**
		 * Encode given character array into base64 string
		 */
		std::string base64_encode(const std::vector<uchar>& bytes_to_encode, unsigned int len);

		/**
		 * Decode given character array from base64 string
		 */
		std::string base64_decode(std::string const& s);

		/**
		 * Encode given image into base64 string
		 */
		std::string base64_encode_image(const cv::Mat &image);

		/**
		 * Decode an image from base64 string
		 */
		cv::Mat base64_decode_image(std::string const& s);
    }
}
