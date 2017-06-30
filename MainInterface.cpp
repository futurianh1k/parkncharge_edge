// File:	MainInterface.cpp
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

#include "MainInterface.h"

#include "Utils.h"
#include "IOUtils.h"
#include "CascadeClassifier.h"
#include "ServerSyncMessage.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <glog/logging.h>

using namespace seevider;
namespace pt = boost::posix_time;

using cv::Mat;

MainInterface::MainInterface() :
	mOperation(true),
	mSettings(std::make_shared<Settings>(SYSTEM_FOLDER_CORE + "settings.ini"))
{
	LOG(INFO) << "Starting the main activity";

	// First retrieve network addresses
	retrieveNetworkAddresses();

	if (!mSettings->loadSettings()) {
		LOG(FATAL) << "Failed to load settings";
		mOperation = false;
		return;
	}
	
	// Constrauct the parking spot manager
	mParkingSpotManager = std::make_shared<ParkingSpotManager>();

    // Construct core resource instances
	mVideoReader = std::make_shared<SerialVideoReader>();
	mServMsgQueue = std::make_shared<MessageQueue>();

	// Open the connected camera
	//mVideoReader->open("..\\..\\Video\\20160527120257(fixed).avi");
	mVideoReader->open(0);

    //--------------------------------
	// Start the resource managing threads.
	//--------------------------------

	// Construct HTTP uploader
	mHTTPServUploader = std::make_unique<ServerNetworkHandler>(mServMsgQueue,
		std::dynamic_pointer_cast<SensorInfo, Settings>(mSettings), mSettings->ServerDataFilename);

	// Construct TCP socket listener
	mTCPSocketListener = std::make_unique<TCPSocketListener>(mParkingSpotManager, mVideoReader, mSettings, mMutualConditionVariable);

	if (mSettings->Type == CLASSIFIER_CASCADE) {
		mDetector = std::make_unique<CascadeClassifier>(SYSTEM_FOLDER_CORE + mSettings->TrainedFilename);
	}

    // Share message queues
    ParkingSpot::setMessageQueue(mServMsgQueue);
    ParkingSpot::setVideoReader(mVideoReader);

	cv::namedWindow(mDebugWindowName);

	// Send an initial sync message
	/*std::unique_ptr<IMessageData> sync_data = std::make_unique<ServerSyncMessage>(mVideoReader->size(),
	boost::posix_time::second_clock::local_time(), getJSONParkingSpots());
	mServMsgQueue->push(sync_data);*/
}

MainInterface::~MainInterface() {
	LOG(INFO) << "Finishing the main activity";

	// Destroy the debug window
	cv::destroyWindow(mDebugWindowName);

	// Release resources
	mHTTPServUploader->destroy();
	mTCPSocketListener->destroy();
	mVideoReader->close();
}

void MainInterface::run() {
    int inputKey = 0;
    Mat frame, output;
	pt::ptime now;
	int dummyID = 1;

	if (!mVideoReader->isOpened()) {
		// if video is not opened, there is nothing to do with it.
		// Maybe we need to upload some message to server that shows
		// the sensor has some problem to connect the camera.
		return;
	}

	while (!mVideoReader->isReady()) {
		// Wait until the camera connection becomes ready
		boost::this_thread::sleep_for(boost::chrono::seconds(1));
	}

	while (mOperation && mVideoReader->read(frame, now)) {
		if (mMutualConditionVariable.ManagementMode) {
			// Wait until the modifier changes settings
			LOG(INFO) << "Enter to the management mode";
			boost::mutex::scoped_lock lockEnter(mMutualConditionVariable.MutexEntrace);
			boost::mutex::scoped_lock lockExit(mMutualConditionVariable.MutexExit);

			lockEnter.unlock();
			mMutualConditionVariable.SenderCV.notify_all();
			mMutualConditionVariable.ReceiverCV.wait(lockExit);
			LOG(INFO) << "Exit from the management mode";
		}
		else {
			// Draw current status
			output = drawParkingStatus(frame);

			// do something
			cv::putText(output, mIPv4Address, cv::Point(0, frame.rows), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0));
			cv::imshow(mDebugWindowName, output);
			inputKey = 0xFF & cv::waitKey(30);

			// Update parking spots with current frame
			updateSpots(frame, now);

			if (inputKey == 27) {
				break;
			}
			else if (inputKey == 'i' || inputKey == 'I') {
				// (Re)initialize parking spots.
				initParkingSpots();
			}

			if (inputKey >= '1' && inputKey <= mParkingSpotManager->size() + '0') {
				// Add a timer for testing purpose
				int idx = inputKey - '1';
				if ((*mParkingSpotManager)[idx]->isOccupied()) {
					(*mParkingSpotManager)[idx]->exit(frame, now);
				}
				else {
					std::cout << "Timer " << std::to_string(idx) << " begins" << std::endl;
					(*mParkingSpotManager)[idx]->enter(frame, now);
				}
			}
			else if (inputKey == 's' || inputKey == 'S') {
				std::unique_ptr<IMessageData> sync_data = std::make_unique<ServerSyncMessage>(mVideoReader->size(),
					boost::posix_time::second_clock::local_time(), mParkingSpotManager->toPTree());
				mServMsgQueue->push(sync_data);
			}
		}
    }
}

void MainInterface::retrieveNetworkAddresses() {
	using boost::asio::ip::tcp;

	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	std::string h = boost::asio::ip::host_name();
	tcp::resolver::iterator end;
	tcp::resolver::iterator endpoint_iterator = resolver.resolve({ h, "" });

	DLOG(INFO) << "Host name is " << h;
	while (endpoint_iterator != end) {
		auto addr = endpoint_iterator->endpoint().address();
		if (addr.is_v4()) {
			mIPv4Address = addr.to_string();
			LOG(INFO) << "IPv4: " << mIPv4Address;
		}
		else if (addr.is_v6()) {
			mIPv6Address = addr.to_string();
			LOG(INFO) << "IPv6: " << mIPv6Address;
		}
		else {
			LOG(INFO) << addr.to_string();
		}
		endpoint_iterator++;
	}

}

void MainInterface::initParkingSpots()
{
	int id = -1, timeLimit = 10;
	int key, outputSequence = 0;
	ROISettingCallbackData callbackData;
	int64 tic;
	pt::ptime now;
	Mat frame, clonedFrame;
	std::stringstream outputFilenameBuilder;
	cv::Rect roi;

	mVideoReader->read(frame, now);
	clonedFrame = frame.clone();

	callbackData.window_name = mInitializeWindow;
	callbackData.image = frame.clone();

	cv::namedWindow(mInitializeWindow);
	cv::setMouseCallback(mInitializeWindow, ROISetting_OnMouse, &callbackData);

	// Print usage
	print_usage_roi_settings();

	do {
		// Update image window
		callbackData.roi_set = false;
		clonedFrame = frame.clone();
		drawParkingStatus(clonedFrame);
		for (const auto &spot : *mParkingSpotManager) {
			rectangle(clonedFrame, spot.second->ROI, CV_RGB(255, 0, 0));
		}
		imshow(mInitializeWindow, clonedFrame);

		key = cv::waitKey();

		// Do nothing when a user is drawing a rectangle
		if (callbackData.start_draw)
			continue;

		switch (key)
		{
		case 'r':	// Update the occupancy status.
		case 'R':
			tic = cv::getTickCount();
			updateSpots(frame, now);
			std::cout << "Occupancy update took " << (cv::getTickCount() - tic) * 1000 / cv::getTickFrequency() << " milliseconds." << std::endl;

			break;

		case 'f':	// Reload frame
		case 'F':
			mVideoReader->read(frame, now);

			break;
			
		case 'a':	// Add a parking spot
		case 'A':
			if (!callbackData.roi_set)
				break;

			std::cout << "Please enter ID of the new spot: ";
			std::cin >> id;
			std::cout << "Please enter time limit of the new spot (in seconds): ";
			std::cin >> timeLimit;

			roi = cv::Rect(callbackData.roi_x0 < callbackData.roi_x1 ? callbackData.roi_x0 : callbackData.roi_x1,
				callbackData.roi_y0 < callbackData.roi_y1 ? callbackData.roi_y0 : callbackData.roi_y1,
				abs(callbackData.roi_x1 - callbackData.roi_x0), abs(callbackData.roi_y1 - callbackData.roi_y0));

			mParkingSpotManager->add(id, "spot" + std::to_string(id), timeLimit, roi, POLICY_TIMED);

			break;

		case 'e':	// Remove all parking spots.
		case 'E':
			mParkingSpotManager->clear();

			break;

		case 'b':	// Remove the most recently added parking spot
		case 'B':

			break;

		default:

			break;
		}
	} while (key != 27);

	cv::destroyWindow(mInitializeWindow);
}

void MainInterface::updateSpots(const Mat &frame, const pt::ptime& now) {
	using std::vector;
	using cv::Rect;
	using cv::Point;

	if (frame.empty()) {
		return;
	}

	if (mParkingSpotManager->size() >= 1) {
		if (mSettings->MotionDetectionEnabled) {
			//detectMotion(frame);
		}

		for (auto &elem : *mParkingSpotManager) {
			vector<Rect> locs;
			std::shared_ptr<ParkingSpot> &parkingSpot = elem.second;

			if (!parkingSpot->UpdateEnabled) {
				// Update only if the status of the parking spot is unstable
				continue;
			}

			Mat croppedFrame = frame(parkingSpot->ROI);

			if (parkingSpot->update(mDetector->detect(croppedFrame, locs), mSettings->MotionDetectionEnabled)) {
				if (parkingSpot->isOccupied()) {
					// if the status has changed to 'Occupied' from 'Empty'
					parkingSpot->enter(frame.clone(), now);
				}
				else {
					// if the status has changed to 'Empty' from 'Occupied'
					parkingSpot->exit(frame.clone(), now);
				}
			}
		}
	}
}

Mat MainInterface::drawParkingStatus(const Mat& frame) const  {
	Mat drawn = frame.clone();
	
	for (auto &parkingSpot : *mParkingSpotManager) {
		cv::Scalar color;
		if (parkingSpot.second->isOccupied()) {
			color = CV_RGB(255, 0, 0);
		}
		else {
			color = CV_RGB(0, 255, 0);
		}

		cv::rectangle(drawn, parkingSpot.second->ROI, color, 2);
	}

	return drawn;
}

void MainInterface::print_usage_roi_settings()
{
	std::cout << "Usage:\n" <<
		"(r) run algorithm on given ROIs, (a) add drawn rectangle as a parking spot,\n"
		"(e) remove all parking spots, (b) remove recently added parking spot,\n" <<
		"(ESC) close the initialization session and run the program" << std::endl;
}
