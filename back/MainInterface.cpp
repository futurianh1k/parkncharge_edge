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

#include <ctime>

using namespace seevider;
namespace pt = boost::posix_time;

using cv::Mat;


PyObject* create_detector(PyObject* p,  const char* model_path, const char* labelmap_file, double confidence, double label);
PyObject* inference_image(PyObject* p,  cv::Mat image, PyObject* localizer, PyObject* recognizer);
int null_inference(PyObject* p, const char* image_path, PyObject* localizer, PyObject* recognizer);


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
    // shared memory ?
	mVideoReader = std::make_shared<SerialVideoReader>(std::dynamic_pointer_cast<CameraInfo, Settings>(mSettings));
	mServMsgQueue = std::make_shared<MessageQueue>();
	// Open the connected camera
	//mVideoReader->open("..\\..\\Video\\20160527120257(fixed).avi");
    //open device
	mVideoReader->open(0);
	//mVideoReader->open(1);
	//mVideoReader->open(2);
	//mVideoReader->open(3);
	//mVideoReader->open(4);
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

// LPR engine initialize
	Py_Initialize(); // Python interpreter init
// * 1) Load share objects detector.so
	PyRun_SimpleString("import sys, os, imp");
        PyRun_SimpleString("sys.path.append(os.getcwd())");
	p_detector = PyImport_ImportModule("detector");
	
	if(p_detector == NULL) //Object creation error
	{
		Py_XDECREF(p_detector);
		LOG(INFO) << "Cannot import detector";
	}
	else
	{
// * 2) Initialize Localizer SSD(Single Shot Multibox Detector)
		localizer = create_detector(p_detector, LOCALIZER_PATH, L_LABEL_MAP, L_CONF, L_LABEL);
		if(localizer==NULL)
		{
			Py_XDECREF(localizer);
			LOG(INFO) << "localizer is NULL";
		}

// * 3) Initialize Recognizer SSD
		recognizer = create_detector(p_detector, RECOGNIZER_PATH, R_LABELMAP, R_CONF, R_LABEL);
		if(recognizer==NULL)
		{
			LOG(INFO) << "recognizer is NULL";
			Py_XDECREF(recognizer);
		}
		Py_XDECREF(p_detector);
		p_inference = PyImport_ImportModule("inference_mobilenet_tf");
		if(p_inference == NULL) // Object creation error
		{
			LOG(INFO) << "Cannot import inference_mobilenet_tf";
			Py_XDECREF(p_inference);
		}
		else
		{
// * 4) Initialize GPU with null image
			tmp = null_inference(p_inference, NULL_PATH, localizer, recognizer);
			if(tmp)
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

MainInterface::~MainInterface() {

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
			//LOG(INFO) << "checkpoint a6" ;

	while (!mVideoReader->isReady()) {
		// Wait until the camera connection becomes ready
		boost::this_thread::sleep_for(boost::chrono::seconds(1));
	}
			//LOG(INFO) << "checkpoint a7" ;

	// Motion detector--delayed initialization to retrieve the original frame size
#if 1
	mMotionDetector = std::make_unique<MotionDetection>(mVideoReader->size());
			//LOG(INFO) << "checkpoint a8" ;
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
#endif
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
	int id = -1, timeLimit = 10, policy = 5; //hard coding
	std::string spotName="spot";
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
	char* plate_str;
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
			       	}
			}

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
					// if (mLPDetector->detect(croppedFrame, plates)) {
					// 	int max = 0;
					// 	int index;
					// 	for (size_t i = 0; i < plates.size(); i++) // get the index of the largest plate detected
					// 	{										   // in case multiple plates are detected in a single vehicle
					// 		if (max < plates[i].width * plates[i].height)
					// 		{
					// 			max = plates[i].width * plates[i].height;
					// 			index = i;
					// 		}
					// 	}
					// 	lpFrame = croppedFrame(plates[index]);
					// 	//imshow("LP", lpFrame);
					// }
					// std::string PN = "null";
					// /*if (mLPR != nullptr) {
					// 	PN = mLPR->recognize(croppedFrame);
					// }*/
                    			//=====================================================
					lpFrame = parkingSpot->enter(frame.clone(), parkingSpot->ROI, now,"QISENS6"); // QISENS6 IS PLATE NUMBER SO WHEN YOU GET PLATE NUMBER BY DETECTION ALGORITHM YOU HAVE TO REPLACE
					imwrite("lpFrame.jpg", lpFrame);
					printf("*******parkingSpot ok**********\n");
					//jeeeun
//-------------------------------------------------------------------------------------------------------------------
										
					result = inference_image(p_inference, lpFrame, localizer, recognizer);
					printf("finish inference func\n");
					if (PyUnicode_Check(result)) 
					{
						PyObject * temp_bytes = PyUnicode_AsEncodedString(result, "UTF-8", "strict"); // Owned reference
						if (temp_bytes != NULL) {
							Py_XDECREF(result);
							plate_str = PyBytes_AS_STRING(temp_bytes); // Borrowed pointer
							plate_str = strdup(plate_str);
							std::cout << "plate : " << plate_str << std::endl;
							printf("plate : %s\n", plate_str);
							Py_DECREF(temp_bytes);
							}   
						else 
						{
							Py_XDECREF(result);
							printf("encoding error!\n");
						}
					}
////-------------------------------------------------------------------------------------------------------------------							
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


/// localizer, recognizer function definition

PyObject* create_detector(PyObject* p_detector, const char* model_path, const char* labelmap_file, double confidence, double label)
{
	PyObject* init_detector;
	PyObject* detector;
	if(p_detector)
	{
		init_detector =PyObject_GetAttrString(p_detector,"create_detector");
		if(init_detector)
		{
			detector = PyObject_CallFunction(init_detector, "ssdd", model_path, labelmap_file, confidence, label);
			if(detector)
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

/***********lpr_recognizer_detect*************************************************
* Getting plate number from recognizer detection
* Arguments
- p_detector : LPR library Object
- ios_threshold : Intersection Over Small ( overlap area/smaller box area of 2 boxes)
- iou_threshold : intersection over union threshold (overlap area/union area of 2 boxes)
	refer to <http://www.pyimagesearch.com/2016/11/07/intersection-over-union-iou-for-object-detection/>
* Return
- multi_plates object as a PyObject* type
*******************************************************************************/
PyObject* inference_image(PyObject* p_inference, cv::Mat image, PyObject* localizer, PyObject* recognizer)
{
	import_array();
	printf("inference_image function start!!!****\n");
    char timebuf[100];
    sprintf(timebuf, "cpp_main infer..._%d.jpg", int(time(0)/60));
    cv::imwrite(timebuf, image);
    printf("image shape : %d %d", image.rows, image.cols);   
    PyObject* init_inference_image;
	PyObject* inference;
	// mat -> numpy
	int cropImage_len = image.total() * image.elemSize();
	uchar* m = new uchar[cropImage_len];
	std::memcpy(m, image.data, cropImage_len * sizeof(uchar));
	npy_intp mdim[] = {image.rows, image.cols, 3};
	PyObject* matimg = PyArray_SimpleNewFromData(3, mdim, NPY_UINT8, (void*) m);
	printf("mat to numpy array finish!****\n");
	if(p_inference)
	{
		init_inference_image = PyObject_GetAttrString(p_inference, "inference_an_image_simple");
		if(init_inference_image)
		{
			printf("pyobject function call!!! ********\n");
            PyObject* args = Py_BuildValue("OOO", matimg, localizer, recognizer);
            inference = PyObject_CallObject(init_inference_image, args);
            //inference = PyObject_CallFunction(init_inference_image, "OOO", matimg, localizer, recognizer);
			if(inference)
			{
				Py_XDECREF(init_inference_image);
				return inference;
			}
			else
			{
				Py_XDECREF(inference);
				LOG(INFO) << "Can't call inference_an_image_simple";
				return 0;
			}
		}
		else
		{
			Py_XDECREF(init_inference_image);
			LOG(INFO) << "Can't load inference_an_image_simple function";
			return 0;
		}
	}
	LOG(INFO) << "inference_image load : Can't load inference_mobilenet_tf library";
	return NULL;
}
int null_inference(PyObject* p, const char* image_path, PyObject* localizer, PyObject* recognizer)
{
	PyObject* init_inference;
	PyObject* inference;
	if(p)
	{
		init_inference = PyObject_GetAttrString(p, "null_inference");
		if(init_inference)
		{
			inference = PyObject_CallFunction(init_inference, "sOO", image_path, localizer, recognizer);
			if(inference)
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
