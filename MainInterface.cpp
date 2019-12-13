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
//#include "CascadeClassifier.h"
#include "ServerSyncMessage.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string.h>

#include <glog/logging.h>
#include "Config.h"
//#include "ImageNetClassifier.h"

#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

using namespace seevider;
namespace pt = boost::posix_time;

using cv::Mat;
char f_path[] = {"./image"};

//PyObject *create_detector(PyObject *p, const char *model_path, const char *labelmap_file, double confidence, double label);
PyObject *create_detector(PyObject *p, const char *model_path, const char *labelmap_file, int mode);
//PyObject* inference_image(PyObject* p,  const char*, PyObject* localizer, PyObject* recognizer);
//int null_inference(PyObject *p, const char *image_path, PyObject *localizer, PyObject *localizer2, PyObject *recognizer);
int null_inference(PyObject *p, const char *image_path, PyObject *localizer2);
PyObject *call_localizer_Vehicle(PyObject *, char *, PyObject *);
//PyObject *call_localizer(PyObject *, char *, PyObject *);
//PyObject *call_recognizer(PyObject *, char *, char *, PyObject *);
//bool localizer_detect(char *, char *);
bool localizer_detect(std::vector<cv::Rect>, cv::Rect);

MainInterface::MainInterface() : mOperation(true),
								 //mLightOn(false),
								 mSettings(std::make_shared<Settings>(SYSTEM_FOLDER_CORE + "settings.ini"))
{

	//=================삭제할부분 === make directory
	int nResult = mkdir(f_path, 0777);
	if (nResult == -1)
		printf("image folder already exist\n");
	//===========================================

	LOG(INFO) << "Starting the main activity";

	// First retrieve network addresses
	retrieveNetworkAddresses();

	if (!mSettings->loadSettings())
	{
		LOG(FATAL) << "Failed to load settings";
		mOperation = false;
		return;
	}

	// Constrauct the parking spot manager
	mParkingSpotManager = std::make_shared<ParkingSpotManager>();

	// Construct core resource instances
	// shared memory ?
	mVideoReader = std::make_shared<SerialVideoReader>(std::dynamic_pointer_cast<CameraInfo, Settings>(mSettings));
	mServMsgQueue = std::make_shared<MessageQueue>();

	// **************************************************************************
	//jeeeun mod camera or video

	// Open the connected camera

	std::string jsonFilename = mSettings->VideoFilename;
	int filenameTOint = atoi(jsonFilename.c_str());

	//filnname int => camera : return true
	//filename (str) => video : return false
	bool mIsVideoFile = (filenameTOint == 0 && jsonFilename.compare("0"));

	if (mIsVideoFile)
	{
		//video
		//std::cout << "Video version" << std::endl;
		std::cout << "Video Filename : " << jsonFilename << std::endl;
		mVideoReader->open(jsonFilename);
		// ok
	}
	else
	{
		//camera
		//std::cout << "Camera Version" << std::endl;
		std::cout << "Camera USB port number : " << filenameTOint << std::endl;
		mVideoReader->open(filenameTOint);
	}

	// ***************************************************************************

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

	// LPR engine initialize
	Py_Initialize(); // Python interpreter init
					 // * 1) Load share objects detector.so
	PyRun_SimpleString("import sys, os, imp");
	PyRun_SimpleString("sys.path.append(os.getcwd())");
	p_detector = PyImport_ImportModule("detector");

	if (p_detector == NULL) //Object creation error
	{
		Py_XDECREF(p_detector);
		LOG(INFO) << "Cannot import detector";
	}
	else
	{
		localizer_v = create_detector(p_detector, LOCALIZER_V_PATH, L_LABEL_MAP_V, 2);
		if (localizer_v == NULL)
		{
			Py_XDECREF(localizer_v);
			LOG(INFO) << "vehicle localizer is NULL";
		}

		Py_XDECREF(p_detector);
		p_inference = PyImport_ImportModule("inference_mobilenet_tf");
		if (p_inference == NULL) // Object creation error
		{
			LOG(INFO) << "Cannot import inference_mobilenet_tf";
			Py_XDECREF(p_inference);
		}
		else
		{
			tmp = null_inference(p_inference, NULL_PATH, localizer_v);
			if (tmp)
				LOG(INFO) << "Finishing the python initialize activity";
		}
	}

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

MainInterface::~MainInterface()
{
	Py_XDECREF(localizer_v);
	Py_XDECREF(p_inference);
	Py_Finalize();
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

void MainInterface::run()
{
	int inputKey = 0;
	Mat frame, output;
	pt::ptime now;
	int dummyID = 1;
	if (!mVideoReader->isOpened())
	{
		// if video is not opened, there is nothing to do with it.
		// Maybe we need to upload some message to server that shows
		// the sensor has some problem to connect the camera.
		// std::cout << "return1" << std::endl;

		return;
	}
	// LOG(INFO) << "checkpoint a6" ;

	while (!mVideoReader->isReady())
	{
		// Wait until the camera connection becomes ready

		boost::this_thread::sleep_for(boost::chrono::seconds(1));
	}
	// LOG(INFO) << "checkpoint a7" ;

	// Motion detector--delayed initialization to retrieve the original frame size
#if 1
	//	mMotionDetector = std::make_unique<MotionDetection>(mVideoReader->size());
	//LOG(INFO) << "checkpoint a8" ;
	while (mOperation && mVideoReader->read(frame, now))
	{
		if (mMutualConditionVariable.ManagementMode)
		{
			// Wait until the modifier changes settings
			LOG(INFO) << "Enter to the management mode";
			boost::mutex::scoped_lock lockEnter(mMutualConditionVariable.MutexEntrace);
			boost::mutex::scoped_lock lockExit(mMutualConditionVariable.MutexExit);

			lockEnter.unlock();
			mMutualConditionVariable.SenderCV.notify_all();
			mMutualConditionVariable.ReceiverCV.wait(lockExit);
			LOG(INFO) << "Exit from the management mode";
		}
		else
		{
			if (frame.empty())
			{
				if (mVideoReader->framelen() == mVideoReader->framenow()) //비디오가 끝난거면
					std::cout << "VIDEO IS FINISH" << std::endl;
				else
					LOG(INFO) << "Frame is empty";
				return;
			}
			//LOG(INFO) << "checkpoint a9";
			// Draw current status
			output = mParkingSpotManager->drawParkingStatus(frame);

			// do something
			cv::putText(output, mIPv4Address, cv::Point(0, frame.rows), cv::FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255, 0, 0));
#ifdef DEBUG
			cv::imshow(mDebugWindowName, output);
#endif
			//LOG(INFO) << "checkpoint aa" ;
			inputKey = 0xFF & cv::waitKey(30);

			// Detect motions
			//LOG(INFO) << "checkpoint ab" ;
			//Ken, could not fix this - "update"
			//mMotionDetector->update(frame);

			//LOG(INFO) << "checkpoint ac" ;

			// Check if any motion has detected in this frame
			/*			if (mMotionDetector->isMotionDetected(frame)) {
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
*/
			// Update parking spots with current frame
			updateSpots(frame, now);

#ifdef DEBUG
			if (inputKey == 27)
			{
				break;
			}
			else if (inputKey == 'i' || inputKey == 'I')
			{
				// (Re)initialize parking spots.
				initParkingSpots();
			}

			// For debugging
			if (inputKey >= '1' && inputKey <= mParkingSpotManager->size() + '0')
			{
				// Add a timer for testing purpose
				int idx = inputKey - '1';
				if ((*mParkingSpotManager)[idx]->isOccupied())
				{
					(*mParkingSpotManager)[idx]->exit(frame, now, "QISENS6");
				}
				else
				{
					std::cout << "Timer " << std::to_string(idx) << " begins" << std::endl;
					//(*mParkingSpotManager)[idx]->enter(frame, now);
				}
			}
			else if (inputKey == 's' || inputKey == 'S')
			{
				std::unique_ptr<IMessageData> sync_data = std::make_unique<ServerSyncMessage>(mVideoReader->size(),
																							  boost::posix_time::second_clock::local_time(), mParkingSpotManager->toPTree());
				mServMsgQueue->push(sync_data);
			}
#endif
		}
	}
#endif
}

void MainInterface::retrieveNetworkAddresses()
{
	using boost::asio::ip::tcp;

	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	std::string h = boost::asio::ip::host_name();
	tcp::resolver::iterator end;
	tcp::resolver::iterator endpoint_iterator = resolver.resolve({h, ""});

	DLOG(INFO) << "Host name is " << h;
	while (endpoint_iterator != end)
	{
		auto addr = endpoint_iterator->endpoint().address();
		if (addr.is_v4())
		{
			mIPv4Address = addr.to_string();
			LOG(INFO) << "IPv4: " << mIPv4Address;
		}
		else if (addr.is_v6())
		{
			mIPv6Address = addr.to_string();
			LOG(INFO) << "IPv6: " << mIPv6Address;
		}
		else
		{
			LOG(INFO) << addr.to_string();
		}
		endpoint_iterator++;
	}
}

void MainInterface::initParkingSpots()
{
	int id = -1, timeLimit = 5, policy = 5; //hard coding
	std::string spotName = "spot";
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

	do
	{
		// Update image window
		callbackData.roi_set = false;
		clonedFrame = mParkingSpotManager->drawParkingStatus(frame);
		for (const auto &spot : *mParkingSpotManager)
		{
			rectangle(clonedFrame, spot.second->ROI, CV_RGB(255, 0, 0));
		}
		imshow(mInitializeWindow, clonedFrame);
		key = cv::waitKey();

		// Do nothing when a user is drawing a rectangle
		if (callbackData.start_draw)
			continue;

		switch (key)
		{
		case 'r': // Update the occupancy status.
		case 'R':
			tic = cv::getTickCount();
			updateSpots(frame, now);
			std::cout << "Occupancy update took " << (cv::getTickCount() - tic) * 1000 / cv::getTickFrequency() << " milliseconds." << std::endl;

			break;

		case 'f': // Reload frame
		case 'F':
			mVideoReader->read(frame, now);

			break;

		case 'a': // Add a parking spot
		case 'A':
			if (!callbackData.roi_set)
				break;

			std::cout << "Please enter <<ID>> of the new spot : ";
			std::cin >> id;
			//std::cout << "Please enter <<time limit>> of the new spot (in seconds) : ";
			//std::cin >> timeLimit;
			//std::cout << "Please enter <<spot name>> of the new spot : ";
			//std::cin >> spotName;
			//std::cout << "Please enter <<policy ID>> of the new spot : ";
			//std::cin >> policy;

			roi = cv::Rect(callbackData.roi_x0 < callbackData.roi_x1 ? callbackData.roi_x0 : callbackData.roi_x1,
						   callbackData.roi_y0 < callbackData.roi_y1 ? callbackData.roi_y0 : callbackData.roi_y1,
						   abs(callbackData.roi_x1 - callbackData.roi_x0), abs(callbackData.roi_y1 - callbackData.roi_y0));

			mParkingSpotManager->add(id, "spot" + std::to_string(id), timeLimit, roi, POLICY_TIMED);

			break;

		case 'e': // Remove all parking spots.
		case 'E':
			mParkingSpotManager->clear();

			break;

		case 'b': // Remove the most recently added parking spot
		case 'B':

			break;

		default:

			break;
		}
	} while (key != 27);
	cv::destroyWindow(mInitializeWindow);
#endif
}

void MainInterface::updateSpots(const Mat &frame, const pt::ptime &now)
{
	using cv::Point;
	using cv::Rect;
	using std::vector;
	char *plate_str;

	char timebuf[100];
	sprintf(timebuf, "./image/cpp1_1%d.jpg", int(time(0)));
	imwrite(timebuf, frame);
	clock_t starttime = clock();
	cv::Rect tmp;
	vector<Rect> car;
	// PyListObject* labellist, *lerror, *lbbox;
	PyObject *labellist, *lerror, *lbbox;

	PyObject *result_v = call_localizer_Vehicle(p_inference, timebuf, localizer_v);

	//get return value as pyobject
	if (!PyArg_ParseTuple(result_v, "OO", &labellist, &lbbox))
		LOG(INFO) << "*can't convert python val to c - vehicle*\n";


	Mat clonedFrame = frame.clone();

	// std::cout << "C size : "<<PyList_Size(lbbox) << std::endl;
	// return type is list.
	// std::cout << "====================tmp==================" << std::endl;
	for (int i = 0; i < PyList_Size(lbbox); i++)
	{
		PyObject *btuple = PyList_GetItem(lbbox, (Py_ssize_t)i); // each list has tuple(x1, y1, width, height)
		if (!PyArg_ParseTuple(btuple, "iiii", &tmp.x, &tmp.y, &tmp.width, &tmp.height))
			LOG(INFO) << "* Can't parse python tuple";
		car.push_back(tmp);
		// std::cout << "car " << car[i].x << " " << car[i].y << " " << car[i].width << " " << car[i].height << std::endl;
		rectangle(clonedFrame, tmp, CV_RGB(255, 0, 0));
	}
	imshow("detect box", clonedFrame); 
	// std::cout << "======================================" << std::endl;

	// cv::waitKey(0);
	// std::cout << "inference time : " << (clock() - starttime)/CLOCKS_PER_SEC  << "." << (clock() - starttime)%CLOCKS_PER_SEC << std::endl;
	// std::cout << "ok "<< std::endl; // for debug


	if (mParkingSpotManager->size() >= 1)
	{
		for (auto &elem : *mParkingSpotManager)
		{
			vector<Rect> locs;
			vector<Rect> plates;
			std::shared_ptr<ParkingSpot> &parkingSpot = elem.second;
			// Mat croppedFrame = frame(parkingSpot->ROI);
			if (parkingSpot->update(localizer_detect(car, parkingSpot->ROI)))
			{
				if (parkingSpot->isOccupied())
				{
					//jeeeun
					//parkingSpot->enter(frame.clone(), parkingSpot->ROI, now, parkingSpot->getPlateNumber());
					parkingSpot->enter(frame.clone(), parkingSpot->ROI, now, "null");
					// std::cout << std::endl << "car brand in cpp parsing : " << carbrand << std::endl << std::endl;
				}
				else
				{
					// if the status has changed to 'Empty' from 'Occupied'
					//parkingSpot->exit(frame.clone(), now, parkingSpot->getPlateNumber());
					parkingSpot->exit(frame.clone(), now, "null");
				}
			}
		}
	}
	//===============file remove==================
	if (remove(timebuf) != 0)
		printf("file cannot remove!\n");
}

void MainInterface::print_usage_roi_settings()
{
	std::cout << "Usage:\n"
			  << "(r) run algorithm on given ROIs, (a) add drawn rectangle as a parking spot,\n"
				 "(e) remove all parking spots, (b) remove recently added parking spot,\n"
			  << "(ESC) close the initialization session and run the program" << std::endl;
}

/// localizer, recognizer function definition

PyObject *create_detector(PyObject *p_detector, const char *model_path, const char *labelmap_file, int mode)
{
	PyObject *init_detector;
	PyObject *detector;
	if (p_detector)
	{
		init_detector = PyObject_GetAttrString(p_detector, "create_detector");
		if (init_detector)
		{
			detector = PyObject_CallFunction(init_detector, "ssi", model_path, labelmap_file, mode);
			if (detector)
			{
				Py_XDECREF(init_detector);
				return detector;
			}
			else
			{
				Py_XDECREF(detector);
				LOG(INFO) << "Can't call detector function";
			}
		}
		LOG(INFO) << "Can't find create_detctor";
	}
	LOG(INFO) << "detector : Can't load detector library";

	return NULL;
}

//int null_inference(PyObject *p, const char *image_path, PyObject *localizer, PyObject *localizer2, PyObject *recognizer) // 번호판, 차
int null_inference(PyObject *p, const char *image_path, PyObject *localizer2) // 번호판, 차
{
	// std::cout << "1111111"<<std::endl;
	PyObject *init_inference;
	PyObject *inference;
	if (p)
	{
		init_inference = PyObject_GetAttrString(p, "null_inference");
		if (init_inference)
		{
			inference = PyObject_CallFunction(init_inference, "sO", image_path, localizer2);
			if (inference)
			{
				Py_XDECREF(init_inference);
				return 1;
			}
			else
			{
				Py_XDECREF(inference);
				LOG(INFO) << "Can't call null_inference";
				return 0;
			}
		}
		else
		{
			Py_XDECREF(init_inference);
			LOG(INFO) << "Can't load null_inference function";
			return 0;
		}
	}
	LOG(INFO) << "inference null load : Can't load inference_mobilenet_tf library";
	return 0;
}

bool localizer_detect(std::vector<cv::Rect> car, cv::Rect ROI)
{
	//1. compare roi size. if detect size is too big, or small, delete it
	//2. if the ROI box has more than 50% intersection size with detect box, then occupy.
	int idx = -1, size_inter = 0;
	float size_max = 0;
	for (int i=0; i<car.size(); i++)
	{
		int xline=0, yline = 0;
		float size_com = (float)(car[i].area()) / ROI.area();
		// std::cout << "car[i].area() " << car[i].area() <<" roi.area() " << ROI.area() << std::endl;
		// std::cout << "car : " << car[i].x << " " << car[i].y <<  " " << car[i].width <<  " " << car[i].height<<std::endl;
		if (size_com < 0.4 || size_com > 1.5)
			continue;
		else
		{
			if(ROI.x > car[i].x && ROI.x < car[i].x + car[i].width)
				xline = car[i].x + car[i].width - ROI.x;
			else if(car[i].x > ROI.x && car[i].x < ROI.x + ROI.width)
				xline = ROI.x + ROI.width - car[i].x;
			else if(ROI.x > car[i].x && ROI.x + ROI.width< car[i].x + car[i].width)
				xline = ROI.width;
			else if(car[i].x > ROI.x && car[i].x + car[i].width < ROI.x + ROI.width)
				xline = car[i].width;
			else
				continue;
			
			if(ROI.y > car[i].y && ROI.y < car[i].y + car[i].height)
				yline = car[i].y + car[i].height - ROI.y;
			else if(car[i].y > ROI.y && car[i].y < ROI.y + ROI.height)
				yline = ROI.y + ROI.height - car[i].y;
			else if(ROI.y > car[i].y && ROI.y + ROI.height < car[i].y + car[i].height)
				yline = ROI.height;
			else if(car[i].y > ROI.y && car[i].y + car[i].height < ROI.y + ROI.height)
				yline = car[i].height;
			else
				continue;

			size_inter = xline * yline;
			// std::cout << "xline:" << xline << "yline:" << yline << std::endl;
			// std::cout << "size_inter:" << size_inter << std::endl;
			if(size_max < size_inter)
			{
				size_max = size_inter;
				idx = i;
			}
		}
	}
	// std::cout << "size_max : " << size_max << std::endl;
	// std::cout << "size_max/ROI : " << size_max/ROI.area() << std::endl;

	if(size_max/ROI.area() > 0.5)
	{
		car.erase(car.begin() + idx);
		return true;
	}
	return false;

	

}
PyObject *call_localizer_Vehicle(PyObject *p_inference, char *image, PyObject *localizer)
{
	// std::cout << "1111111"<<std::endl;
	PyObject *init_inference_image;
	PyObject *inference;
	if (p_inference)
	{
		// std::cout << "222222222"<<std::endl;

		init_inference_image = PyObject_GetAttrString(p_inference, "call_localizer_vehicle");
		if (init_inference_image)
		{
			// std::cout << "333333333"<<std::endl;
			inference = PyObject_CallFunction(init_inference_image, "sO", image, localizer);
			if (inference)
			{
				Py_XDECREF(init_inference_image);
				return inference;
			}
			else
			{
				Py_XDECREF(inference);
				LOG(INFO) << "Can't call vehicle localizer\n";
				return 0;
			}
		}
		else
		{
			Py_XDECREF(init_inference_image);
			LOG(INFO) << "Can't load call_localizer_v function\n";
			return 0;
		}
	}
}