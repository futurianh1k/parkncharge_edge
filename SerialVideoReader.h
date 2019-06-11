// File:	SerialVideoReader.h
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

#include <string>
#include <list>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "types.h"
#include "CameraInfo.h"

namespace seevider {
    class SerialVideoReader {
    public:
        /**
         * Construct the class instance.
         */
		SerialVideoReader(std::shared_ptr<CameraInfo> setting);

        /**
         * Basic destructor. If the video API is still opened, close it.
         */
        ~SerialVideoReader();

		/**
		 * Open a video API from one of connected cameras.
		 * @params id An ID of connected camera.
		 * @returns True if success, and false otherwise.
		 */
		bool open(int id);

		/**
		 * Open a video API from given file.
		 * @params filename A filename of target video. The file must be a vaild video file.
		 * @returns True if success, and false otherwise.
		 */
		bool open(std::string filename);

		/**
		 * Retrieve the latest image from the input source.
		 */
		cv::Mat read() const;

        /**
         * Retrieve the latest image frame from the input source.
         * @returns True if success, and false otherwise.
         */
		bool read(cv::Mat &frame, boost::posix_time::ptime &now);

		/**
		 * Retrieve a frame at given time.
		 * @returns True if success, and false otherwise.
		 */
		bool readAt(cv::Mat &frame, boost::posix_time::ptime &time);

        /**
         * Check if video API is currently opened.
         * @returns True if API is opened, and false otherwise.
         */
        bool isOpened() const;

		/**
		 * Check if the queue is ready which means at least one frame is inserted into the both queue.
		 */
		bool isReady() const;

		/**
		 * Close the video reader if it has connected input source.
		 */
		void close();

		/**
		 * Get frame size
		 */
		cv::Size size() const;

	private:
		/**
		 * The processing thread
		 */
		boost::thread mThread;

		/**
		 * Instance of a video source API.
		 */
		cv::VideoCapture mVideoReader;

		/**
		 * Run video manager if this value is true
		 */
		bool Operation;

		/**
		 * True if the source is video
		 */
		bool mFromVideo;

		/**
		 * The number of maximum frames to be stored on the memory
		 */
		int mMaxFrames = 100;

		/**
		 * The length of seconds to store the key frames.
		 */
		int mHoldingFramesSeconds = 100;

        /**
         * A mutex to limit the simultaneous access to the video API.
         */
        mutable boost::mutex mMutex;

		/**
		 * Set it for how long we will wait until the remaining job processes.
		 * The unit of this number is second.
		 */
		int mWaitSeconds = 2;

		/**
		 * A variable that determines how many seconds images are held.
		 */
		int mDestroySeconds = 3;

		/**
		 * Frame size
		 */
		cv::Size mFrameSize;

		/**
		 * Current codec name
		 */
		char mInputFourCC[4];

		/**
		 * Set to true after 'open' process finishes
		 */
		bool mReady = false;

        /**
         * Queue of taken frames
         */
        std::list<std::pair<boost::posix_time::ptime, cv::Mat>> mFrameQueue;

        /**
         * The frame indexer holds frames of each second, from mHoldingFramesSeconds seconds
		 * ago to the present.
         */
        std::unordered_map<boost::posix_time::ptime, cv::Mat, CustomHash> mFrameIndexer;
		
        /**
         * Main entry of the processing thread
         */
        void run();

		/**
		 * Destroy the video reader thread
		 */
		void destroy();
				
	private:

    };
}
