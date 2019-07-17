// File:	ParkingSpot.h
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
#include <opencv2/opencv.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "MessageQueue.h"
#include "SerialVideoReader.h"




namespace seevider {
    class ParkingSpot {
    public:
        
        /**
         * The read-only rectangular ROI
         */
        const cv::Rect& ROI;

        /**
         * The read-only time limit as minutes
         */
        const int& TimeLimit;
        
        /**
         * The read-only server-synchronized ID of the parking spot
         */
		const int& ID;

		/**
		 * The read-only server-synchronized human-friendly name of the parking spot
		 */
		const std::string& SpotName;

		/**
		 * The read-only pre-defined parking policy
		 */
		const PARKING_SPOT_POLICY& ParkingPolicy;

		/**
		* (read-only) Designate if the system needs to update this parking spot
		*/
		const bool &UpdateEnabled;

	private:
		/**
		 * The original rectangular ROI
		 */
		cv::Rect mROI;

		/**
		 * Time limit as minute
		 */
		int mTimeLimit;

		/**
		 * The server-synchronized ID of the parking spot
		 */
		int mID;

		/**
		 * The server-synchronized human-friendly name of the parking spot
		 */
		std::string mSpotName;

		/**
		 * The pre-defined parking policy
		 */
		PARKING_SPOT_POLICY mParkingPolicy;

		/**
		 * Set to be true if the system needs to update this parking spot
		 */
		bool mUpdateEnabled;

		/**
		 * Must be true if the parking spot is occupied
		 */
		bool mOccupied;

		/**
		 * True if the spot is overstayed
		 */
		bool mOverstayed;

		/**
		 * Plane number of the parked car. Could be null if it failed to read PN.
		 */
		std::string mPlateNumber;

		/**
		 * Frame counter represents the occurence of occupancy events.
		 */
		int mOccupiedFrameCounter = 0;

		/**
		 * Designate if the entry image is uploaded to the server.
		 */
		bool mIsEntryImageUploaded;

		/**
		 * Designate if the exit image is uploaded to the server.
		 */
		bool mIsExitImageUploaded;

		/**
		 * IO Service object for the deadline timer
		 */
		boost::asio::io_service mService;

		/**
		 * Work object for the deadline timer
		 */
		boost::asio::io_service::work mWork;

		/**
		 * Timer thread
		 */
		boost::thread mTimerThread;

		/**
		 * Timer to check if the parking expires
		 */
		boost::asio::deadline_timer mParkingTimer;

		/**
		 * Entry time to compute expiration when the timer is expired.
		 */
		boost::posix_time::ptime mEntryTime;

	public:
        /**
         * Basic constructor.
         * @params entryImage The original image taken at the time of entry.
         *  Only a clone of the image will be stored.
         * @param length Allowed length of time for this parking spot
         */
		ParkingSpot(int id, std::string spotName, const int length, const cv::Rect roi, PARKING_SPOT_POLICY policy);
		
		/**
		 * Basic destructor
		 */
        ~ParkingSpot();

        /**
         * Check if the parking spot is occupied
         */
        	bool isOccupied() const;
		void pullDB();

		/**
		 * Check if the parking spot is occupied and overstayed
		 */
		bool isOverstayed() const;

///////////////////////////////
		void connect_DB();
		int entVehicle(const std::string PN, const std::string entryTime);
		void extVehicle(const std::string PN, const std::string entryTime);
		








///////////////////////////////

        /**
         * Must be called when the vehicle comes in.
         */
		void enter(const cv::Mat& entryImage, const cv::Rect &ROI, const boost::posix_time::ptime &entryTime, const std::string PN = "null");

        /**
         * Must be called when the maximum allowed time has reached.
         */
        void overstayed(const cv::Mat& expiredImage, const boost::posix_time::ptime &exprTime);

        /**
         * Must be called when the vehicle goes out.
         */
        void exit(const cv::Mat& exitImage, const boost::posix_time::ptime &exitTime, const std::string PN);

		/**
		 * Update parking spot attributes. If current spot is occupied and the timer is running,
		 * it will not update the spot and return false.
		 */
		bool update(std::string spotName, int timeLimit, cv::Rect roi, PARKING_SPOT_POLICY policy);

		/**
		 * Update current status. Return true if the status has updated.
		 */
		bool update(bool occupied, bool triggerUpdatability);

		/**
		 * Reset the parking timer and status.
		 */
		void reset();


///////////////////////////////
		void connectDB();


/////////////////////////////

	private:
        /**
         * Main entry of the timer thread
         */
        void runTimer();

        /**
         * Start the parking timer
         */
        void startTimer(int res_timeLimit);
	

        /**
         * Stop the parking timer, if any
         */
        void stopTimer();

        /**
         * Notify a expiration message to the global message queue
         */
        void notifyExpiration();
        //-----------------------------------
        // Static functions and variables
        //-----------------------------------
    public:
        /**
         * Set the shared message queue for server-side communication.
         */
		static void setMessageQueue(std::shared_ptr<MessageQueue> &messageQueue);

        /**
         * Set the camera frame reader for the expiration event.
         */
		static void setVideoReader(std::shared_ptr<SerialVideoReader> &videoReader);

		/**
		 * Set the positive threshold
		 */
		static void setPositiveThreshold(int positiveThreshold);

		/**
		 * Set the negative threshold
		 */
		static void setNegativeThreshold(int negativeThreshold);

    private:
        /**
         * Network message queue for uploading the parking spot status information to the server.
         * Data will be inserted when the status changes
         */
        static std::shared_ptr<MessageQueue> mServerMsgQueue;

        /**
         * Since the parking enforcement is time-critical, each timer needs to acquire the camera
         * frame at the time of an expiration event.
         */
		static std::shared_ptr<SerialVideoReader> mVideoReader;

		/**
		 * The positive threshold to be estimated as an occupied parking spot.
		 */
		static int mPositiveThreshold;

		/**
		 * The negative threshold to be estimated as an unoccupied parking spot.
		 */
		static int mNegativeThreshold;
    };
}
