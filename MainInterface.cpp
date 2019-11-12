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
int null_inference(PyObject *p, const char *image_path, PyObject *localizer, PyObject *localizer2, PyObject *recognizer);
PyObject *call_localizer_Vehicle(PyObject *, char *, PyObject *);
PyObject *call_localizer(PyObject *, char *, PyObject *);
PyObject *call_recognizer(PyObject *, char *, char *, PyObject *);
bool localizer_detect(char *, char *);

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
	// Start computer vision engines
	//--------------------------------
	// Occupancy Detector
	/*	if (mSettings->Type == CLASSIFIER_CASCADE) {
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
*/
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
		// * 2) Initialize Localizer SSD(Single Shot Multibox Detector)
		//		localizer = create_detector(p_detector, LOCALIZER_PATH, L_LABEL_MAP, L_CONF, L_LABEL);
		localizer = create_detector(p_detector, LOCALIZER_PATH, L_LABEL_MAP, 0);
		if (localizer == NULL)
		{
			Py_XDECREF(localizer);
			LOG(INFO) << "localizer is NULL";
		}

		localizer_v = create_detector(p_detector, LOCALIZER_V_PATH, L_LABEL_MAP, 2);

		if (localizer_v == NULL)
		{
			Py_XDECREF(localizer_v);
			LOG(INFO) << "vehicle localizer is NULL";
		}

		// * 3) Initialize Recognizer SSD
		//		recognizer = create_detector(p_detector, RECOGNIZER_PATH, R_LABELMAP, R_CONF, R_LABEL);
		recognizer = create_detector(p_detector, RECOGNIZER_PATH, R_LABELMAP, 1);

		if (recognizer == NULL)
		{
			LOG(INFO) << "recognizer is NULL";
			Py_XDECREF(recognizer);
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
			// * 4) Initialize GPU with null image
			tmp = null_inference(p_inference, NULL_PATH, localizer, localizer_v, recognizer);
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

	Py_XDECREF(localizer);
	Py_XDECREF(recognizer);
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
		return;
	}
	//LOG(INFO) << "checkpoint a6" ;

	while (!mVideoReader->isReady())
	{
		// Wait until the camera connection becomes ready
		boost::this_thread::sleep_for(boost::chrono::seconds(1));
	}
	//LOG(INFO) << "checkpoint a7" ;

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
				if(mVideoReader-> framelen() == mVideoReader->framenow()) //비디오가 끝난거면
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
	int id = -1, timeLimit = 10, policy = 5; //hard coding
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
			std::cout << "Please enter <<time limit>> of the new spot (in seconds) : ";
			std::cin >> timeLimit;
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
	if (mParkingSpotManager->size() >= 1)
	{
		for (auto &elem : *mParkingSpotManager)
		{

			vector<Rect> locs;
			vector<Rect> plates;
			std::shared_ptr<ParkingSpot> &parkingSpot = elem.second;

			/*			if (mSettings->MotionDetectionEnabled && mMotionDetector->isMotionDetected(frame, elem.second->ROI)) {
				std::cout << "Motion detected at spot " << elem.first << std::endl;
			       	if(parkingSpot->UpdateEnabled){
					continue;	// if some motion has detected for this ROI, wait until the region becomes stable
			       	} else {
				       	parkingSpot->reset();
			       	}
			}

			if (!parkingSpot->UpdateEnabled) {
				// Update only if the status of the parking spot is unstable
				continue;
			}
*/
			Mat croppedFrame = frame(parkingSpot->ROI);
			//			cv::resize(croppedFrame, croppedFrame, cv::Size(64, 64)); //64, 64로 지워주는 부분은 필요하지 않을 것 같아서 지움

			//=========juhee========= 수정 필요==============================================================
			// printf("============================\n");
			char timebuf[100];
			sprintf(timebuf, "./image/cpp1_1%d.jpg", int(time(0)));
			imwrite(timebuf, croppedFrame);
			PyObject *result = call_localizer(p_inference, timebuf, localizer);

			char *ob1, *plateType, *carbrand; // 순서대로 이미지 이름, 타입
			float pConf, cConf;
			cv::Rect plate, car;
			if (!PyArg_ParseTuple(result, "ss(iiii)f", &ob1, &plateType, &plate.x, &plate.y, &plate.width, &plate.height, &pConf))
			{ // 순서대로 이미지 이름, 번호판타입, 번호판 roi, conf
				printf("***can't convert python val to c***\n");			
			}
			else
			{
				PyObject *result_v = call_localizer_Vehicle(p_inference, timebuf, localizer_v);
				if (!PyArg_ParseTuple(result_v, "s(iiii)f", &carbrand, &car.x, &car.y, &car.width, &car.height, &cConf))
					printf("***can't convert python val to c - vehicle***\n");
				// printf("check localizer return %s %s %f\n", ob1, plateType, pConf);
				// printf("check localizer return %s %f\n",carbrand, cConf);

				//			printf("check localizer return %s %s\n", ob1, ob2);


				char ob3[30];
				memset(ob3, 0, sizeof(ob3));
				strcpy(ob3, ob1);
				Py_XDECREF(result);
				Py_XDECREF(result_v);

				/*
			localizer_detect 에서는 localizer결과값을 매개변수로 받고 그 결과를 bool로 update에 반환
			이후에 localizer리턴값은 레이블, 플레이트 이미지(혹은 bbox들)를 리턴할 것.
			bbox를 리턴하면 bbox에 따라 박스 그려주고 이미지 자르면 되니까 이미지를 또 리턴할 필요x
			*/
				//======================================
				//jeeeun_if_detector_1
				//			if (parkingSpot->update(mODetector->detect(croppedFrame, locs), mSettings->MotionDetectionEnabled)) {
				//localizer 결과로 해당하는 범위 내의 레이블 값이 나온 경우
				//if(true)로 조건을 주게 되면 모든 detect를 전부 exit()으로 처리 (이유 : parkingSpot->isOccupied()가 false라서)
				//따라서 isOccupied()를 true로 해 줘야 하는 경우를 처리 해 주어야 recognizer를 시작 할 수 있음
				if (parkingSpot->update(localizer_detect(carbrand, plateType)))
				{						
					if (parkingSpot->isOccupied())
					{
						car.x += parkingSpot->ROI.x;
						car.y += parkingSpot->ROI.y;
						plate.x += parkingSpot->ROI.x;
						plate.y += parkingSpot->ROI.y;
						// if the status has changed to 'Occupied' from 'Empty'
						// detect license plate in cropped frame
						// if (mLPDetector->detect(croppedFrame, plates)) {
						//jeeeun
						//-------------------------------------------------------------------------------------------------------------------
						// printf("if enter recognizer!");
						if (strcmp(plateType, "None") != 0) // plate를 찾은 경우
						{
							parkingSpot->pConf = int(pConf * 10000);
							parkingSpot->setLocalizerROI(plate);
							// printf("if enter recognizer!\n");

							PyObject *recog_result = call_recognizer(p_inference, ob1, plateType, recognizer);
									// printf("if enter recognizer!\n");

							if (recog_result != NULL)
							{
								PyObject *temp_bytes = PyUnicode_AsEncodedString(recog_result, "UTF-8", "strict"); // Owned reference
								if (temp_bytes != NULL)
								{
									Py_DECREF(recog_result);
									plate_str = PyBytes_AS_STRING(temp_bytes); // Borrowed pointer
									plate_str = strdup(plate_str);
									// std::cout << "plate : " << plate_str << std::endl;
									parkingSpot->setPlateNumber(plate_str);
									Py_DECREF(temp_bytes);
								}
								else
								{
									Py_XDECREF(temp_bytes);
									Py_XDECREF(result);
									printf("encoding error!\n");
								}
							}
							else
							{
								printf("recognizer can't find!\n");
								Py_XDECREF(recog_result);
							}
						}
						// if (strcmp(carbrand, "None") != 0 && strcmp(carbrand, "50") != 0) // carbrand를 찾은 경우
						if (strcmp(carbrand, "None") != 0 ) // carbrand를 찾은 경우
						{
							parkingSpot->cConf = int(cConf * 10000);
							parkingSpot->setVehicleROI(car);
							int n = atoi(carbrand);
							carbrand = parseToBrand(p_inference, n);
							// if (strcmp(carbrand, "unknown"))
							parkingSpot->setCarBrand(carbrand);
						}
						//jeeeun
						parkingSpot->enter(frame.clone(), parkingSpot->ROI, now, parkingSpot->getPlateNumber());

					} //if occupied()문에 걸려있는 else임
					else
					{
						// if the status has changed to 'Empty' from 'Occupied'
						parkingSpot->exit(frame.clone(), now, parkingSpot->getPlateNumber());
					}
					//				parkingSpot->enter(frame.clone(), parkingSpot->ROI, now, "QISENS6"); // QISENS6 IS PLATE NUMBER SO WHEN YOU GET PLATE NUMBER BY DETECTION ALGORITHM YOU HAVE TO REPLACE
				}
				//===============file remove==================
				if (strcmp(ob3, "None") != 0)
					if (remove(ob3) != 0)
						printf("file cannot remove**\n");
			}
			if (remove(timebuf) != 0)
				printf("file cannot remove!\n");
		}
	}
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

int null_inference(PyObject *p, const char *image_path, PyObject *localizer, PyObject *localizer2, PyObject *recognizer) // 번호판, 차
{
	PyObject *init_inference;
	PyObject *inference;
	if (p)
	{
		init_inference = PyObject_GetAttrString(p, "null_inference");
		if (init_inference)
		{
			inference = PyObject_CallFunction(init_inference, "sOOO", image_path, localizer, localizer2, recognizer);
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
bool localizer_detect(char *carBrand, char *plateType) // 차 혹은 번호판을 인식했을 때로 바꾸기 -> 둘 중 하나라도 인식하면 true임
{
	// if (strcmp(carBrand, "None") != 0 && strcmp(carBrand, "50") != 0) //label 50 : unknown
	// {
	// 	return true;
	// }
	// else if (strcmp(plateType, "None") != 0)
	// 	return true;
	if(strcmp(carBrand, "None") != 0 || strcmp(plateType, "None") != 0)
		return true;
	else
		return false;
}

PyObject *call_recognizer(PyObject *p_inference, char *plateImage, char *plateType, PyObject *recognizer)
{
	PyObject *init_inference;
	PyObject *inference;
	//printf("call_recognizer start!\n");
	if (p_inference)
	{
		init_inference = PyObject_GetAttrString(p_inference, "call_recognizer");
		if (init_inference)
		{
			//printf("enter recognizer if\n");
			inference = PyObject_CallFunction(init_inference, "ssO", plateType, plateImage, recognizer);
			//printf("recognizer callfunction okay\n");
			if (inference)
			{
				Py_XDECREF(init_inference);
				return inference;
			}
			else
			{
				Py_XDECREF(inference);
				printf("Can't call recognizer\n");
				return 0;
			}
		}
		else
		{
			Py_XDECREF(init_inference);
			printf("Can't load call_recognizer function\n");
			return 0;
		}
	}
}

PyObject *call_localizer(PyObject *p_inference, char *image, PyObject *localizer)
{
	PyObject *init_inference_image;
	PyObject *inference;
	if (p_inference)
	{
		init_inference_image = PyObject_GetAttrString(p_inference, "call_localizer");
		if (init_inference_image)
		{
			inference = PyObject_CallFunction(init_inference_image, "sO", image, localizer);
			if (inference)
			{
				Py_XDECREF(init_inference_image);
				return inference;
			}
			else
			{
				Py_XDECREF(inference);
				LOG(INFO) <<"Can't call localizer\n";
				return 0;
			}
		}
		else
		{
			Py_XDECREF(init_inference_image);
			LOG(INFO) <<"Can't load call_localizer function\n";
			return 0;
		}
	}
}

PyObject *call_localizer_Vehicle(PyObject *p_inference, char *image, PyObject *localizer)
{
	PyObject *init_inference_image;
	PyObject *inference;
	if (p_inference)
	{
		init_inference_image = PyObject_GetAttrString(p_inference, "call_localizer_vehicle");
		if (init_inference_image)
		{
			inference = PyObject_CallFunction(init_inference_image, "sO", image, localizer);
			if (inference)
			{
				Py_XDECREF(init_inference_image);
				return inference;
			}
			else
			{
				Py_XDECREF(inference);
				LOG(INFO) <<"Can't call vehicle localizer\n";
				return 0;
			}
		}
		else
		{
			Py_XDECREF(init_inference_image);
			LOG(INFO) <<"Can't load call_localizer_v function\n";
			return 0;
		}
	}
}

char *parseToBrand(PyObject *p, int n)
{
	PyObject *init_inference;
	PyObject *inference;
	char *result;
	if (p)
	{
		init_inference = PyObject_GetAttrString(p, "parseToBrand");
		if (init_inference)
		{
			//PyObject *recog_result = call_recognizer(p_inference, ob1, platetype, recognizer);
			inference = PyObject_CallFunction(init_inference, "i", n);
			if (inference)
			{
				PyObject *temp_bytes = PyUnicode_AsEncodedString(inference, "UTF-8", "strict"); // Owned reference
				if (temp_bytes != NULL)
				{
					result = PyBytes_AS_STRING(temp_bytes); // Borrowed pointer
					result = strdup(result);
					// std::cout << "========================" << std::endl;
					// std::cout << "partoBrand in result : " << result << std::endl;
					// std::cout << "========================" << std::endl;
					Py_DECREF(temp_bytes);
					return result;
				}
				else
				{
					std::cout << "encoding error!" << std::endl;
					return 0;
				}
			}
			else
			{
				Py_XDECREF(inference);
				printf("Can't call parseToBrand\n");
				return 0;
			}
		}
		else
		{
			Py_XDECREF(init_inference);
			printf("Can't load parse function\n");
			return 0;
		}
	}
}
