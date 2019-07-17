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
#include "Config.h"
#include "ImageNetClassifier.h"

using namespace seevider;
namespace pt = boost::posix_time;

using cv::Mat;

MainInterface::MainInterface() :
	mOperation(true),
	mLightOn(false),
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
	mVideoReader = std::make_shared<SerialVideoReader>(std::dynamic_pointer_cast<CameraInfo, Settings>(mSettings));
	mServMsgQueue = std::make_shared<MessageQueue>();
	// Open the connected camera
	//mVideoReader->open("..\\..\\Video\\20160527120257(fixed).avi");
	mVideoReader->open(1);
    //--------------------------------
	// Start the resource managing threads.
	//--------------------------------
	// Construct HTTP uploader
	mHTTPServUploader = std::make_unique<ServerNetworkHandler>(mServMsgQueue,
		std::dynamic_pointer_cast<SensorInfo, Settings>(mSettings), mSettings->ServerDataFilename);
	// Construct TCP socket listener
	mTCPSocketListener = std::make_unique<TCPSocketListener>(mParkingSpotManager, mVideoReader, mSettings, mMutualConditionVariable);
    // Share message queues
    ParkingSpot::setMessageQueue(mServMsgQueue);
    ParkingSpot::setVideoReader(mVideoReader);
    // Initialize threshold of parking spot
    ParkingSpot::setPositiveThreshold(mSettings->ParkingParams.enterCount);
    ParkingSpot::setNegativeThreshold(-mSettings->ParkingParams.exitCount);
	//--------------------------------
	// Start computer vision engines
	//--------------------------------
	// Occupancy Detector
	if (mSettings->Type == CLASSIFIER_CASCADE) {
		mDetector = std::make_unique<CascadeClassifier>(SYSTEM_FOLDER_CORE + mSettings->TrainedFilename, mSettings);
	} else if (mSettings->Type == CLASSIFIER_CNN) {
		mDetector = std::make_unique<ImageNetClassifier>(mSettings);
	}

	mODetector = std::make_unique<IOccupancyDetector>(std::move(mDetector));

	// License Plate Detector
	if (mSettings->Type == CLASSIFIER_CASCADE) {
		mDetector = std::make_unique<CascadeClassifier>(SYSTEM_FOLDER_CORE + mSettings->LPTrainedFilename, mSettings);
	} else if (mSettings->Type == CLASSIFIER_CNN) {
        mDetector = std::make_unique<CascadeClassifier>(SYSTEM_FOLDER_CORE + mSettings->LPTrainedFilename, mSettings);
    }

	mLPDetector = std::make_unique<IPlateDetector>(std::move(mDetector));

	// LPR engine
	/*if (!mSettings->LPRSettingsFilename.empty()) {
		mLPR = std::make_unique<LPR>(mSettings->LPRRegionCode, SYSTEM_FOLDER_CORE + mSettings->LPRSettingsFilename);
	}
	else {
		mLPR = nullptr;
	}*/

	//--------------------------------
	// Start Graphic User Interface
    //--------------------------------
#ifdef DEBUG
	cv::namedWindow(mDebugWindowName);
#endif
	// Send an initial sync message
	/*std::unique_ptr<IMessageData> sync_data = std::make_unique<ServerSyncMessage>(mVideoReader->size(),
	boost::posix_time::second_clock::local_time(), getJSONParkingSpots());
	mServMsgQueue->push(sync_data);*/
}

MainInterface::~MainInterface() {
	LOG(INFO) << "Finishing the main activity";

	// Destroy the debug window
#ifdef DEBUG
	cv::destroyWindow(mDebugWindowName);
#endif

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

	// Motion detector--delayed initialization to retrieve the original frame size
	mMotionDetector = std::make_unique<MotionDetection>(mVideoReader->size());
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
			output = mParkingSpotManager->drawParkingStatus(frame);

			// do something
			cv::putText(output, mIPv4Address, cv::Point(0, frame.rows), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0));
#ifdef DEBUG
			cv::imshow(mDebugWindowName, output);
#endif
			inputKey = 0xFF & cv::waitKey(30);

			// Detect motions
			mMotionDetector->update(frame);

			// Check if any motion has detected in this frame
			if (mMotionDetector->isMotionDetected(frame)) {
				//std::cout << "Motion detected--brighten the light" << std::endl;
				if (!mLightOn) {
					mLight.onMaximum();
				}
				mLightOn = true;
				// TODO: control lighting
			}
			else {
				//std::cout << "No motion detected--dim down the light" << std::endl;
				if (mLightOn) {
					mLight.onDimDown();
				}
				mLightOn = false;
			}

			// Update parking spots with current frame
			updateSpots(frame, now);

#ifdef DEBUG
            if (inputKey == 27) {
				break;
			}
			else if (inputKey == 'i' || inputKey == 'I') {
				// (Re)initialize parking spots.
				initParkingSpots();
			}

			// For debugging
			if (inputKey >= '1' && inputKey <= mParkingSpotManager->size() + '0') {
				// Add a timer for testing purpose
				int idx = inputKey - '1';
				if ((*mParkingSpotManager)[idx]->isOccupied()) {
					(*mParkingSpotManager)[idx]->exit(frame, now,"QISENS6");
				}
				else {
					std::cout << "Timer " << std::to_string(idx) << " begins" << std::endl;
					//(*mParkingSpotManager)[idx]->enter(frame, now);
				}
			}
			else if (inputKey == 's' || inputKey == 'S') {
				std::unique_ptr<IMessageData> sync_data = std::make_unique<ServerSyncMessage>(mVideoReader->size(),
					boost::posix_time::second_clock::local_time(), mParkingSpotManager->toPTree());
				mServMsgQueue->push(sync_data);
			}
#endif
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

#ifdef DEBUG
	callbackData.window_name = mInitializeWindow;
	callbackData.image = frame.clone();

	cv::namedWindow(mInitializeWindow);
	cv::setMouseCallback(mInitializeWindow, ROISetting_OnMouse, &callbackData);

	// Print usage
	print_usage_roi_settings();

	do {
		// Update image window
		callbackData.roi_set = false;
		clonedFrame = mParkingSpotManager->drawParkingStatus(frame);
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
#endif
}

void MainInterface::updateSpots(const Mat &frame, const pt::ptime& now) {
	using std::vector;
	using cv::Rect;
	using cv::Point;

	if (frame.empty()) {
		return;
	}

	if (mParkingSpotManager->size() >= 1) {
		for (auto &elem : *mParkingSpotManager) {
			vector<Rect> locs;
			vector<Rect> plates;
			std::shared_ptr<ParkingSpot> &parkingSpot = elem.second;

			if (mSettings->MotionDetectionEnabled && mMotionDetector->isMotionDetected(frame, elem.second->ROI)) {
				std::cout << "Motion detected at spot " << elem.first << std::endl;
                if(parkingSpot->UpdateEnabled){
                    continue;	// if some motion has detected for this ROI, wait until the region becomes stable
                } else {
                    parkingSpot->reset();
                }}

			if (!parkingSpot->UpdateEnabled) {
				// Update only if the status of the parking spot is unstable
				continue;
			}

			Mat croppedFrame = frame(parkingSpot->ROI);
            cv::resize(croppedFrame, croppedFrame, cv::Size(64, 64));

			if (parkingSpot->update(mODetector->detect(croppedFrame, locs), mSettings->MotionDetectionEnabled)) {
				if (parkingSpot->isOccupied()) {
                    // if the status has changed to 'Occupied' from 'Empty'
					// detect license plate in cropped frame
					Mat lpFrame;
					if (mLPDetector->detect(croppedFrame, plates)) {
						int max = 0;
						int index;
						for (size_t i = 0; i < plates.size(); i++) // get the index of the largest plate detected
						{										   // in case multiple plates are detected in a single vehicle
							if (max < plates[i].width * plates[i].height)
							{
								max = plates[i].width * plates[i].height;
								index = i;
							}
						}
						lpFrame = croppedFrame(plates[index]);
						//imshow("LP", lpFrame);
					}
					std::string PN = "null";
					/*if (mLPR != nullptr) {
						PN = mLPR->recognize(croppedFrame);
					}*/
                    parkingSpot->enter(frame.clone(), parkingSpot->ROI, now,"QISENS6");
				}
				else {
					// if the status has changed to 'Empty' from 'Occupied'
					parkingSpot->exit(frame.clone(), now, "QISENS6");
				}
			}
		}
	}
}

void MainInterface::print_usage_roi_settings()
{
	std::cout << "Usage:\n" <<
		"(r) run algorithm on given ROIs, (a) add drawn rectangle as a parking spot,\n"
		"(e) remove all parking spots, (b) remove recently added parking spot,\n" <<
		"(ESC) close the initialization session and run the program" << std::endl;
}
