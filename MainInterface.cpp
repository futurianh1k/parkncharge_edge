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
	mOperation(true), mManagementMode(false),
	mSettings(std::make_shared<Settings>(SYSTEM_FOLDER_CORE + "settings.ini"))
{
	LOG(INFO) << "Starting the main activity";

	if (!mSettings->loadSettings()) {
		LOG(FATAL) << "Failed to load settings";
		mOperation = false;
		return;
	}

    // Construct core resource instances
	mVideoReader = std::make_shared<SerialVideoReader>();
	mServMsgQueue = std::make_shared<MessageQueue>();

	// Open the connected camera
	//mVideoReader->open("..\\..\\Video\\20160527120257(fixed).avi");
	mVideoReader->open(0);

    // Start the resource managing threads.
	mServNetHandler = std::make_unique<ServerNetworkHandler>(mServMsgQueue, mSettings);

	if (mSettings->Type == CLASSIFIER_CASCADE) {
		mDetector = std::make_unique<CascadeClassifier>(SYSTEM_FOLDER_CORE + mSettings->TrainedFilename);
	}

    // Share message queues
    ParkingSpot::setMessageQueue(mServMsgQueue);
    ParkingSpot::setVideoReader(mVideoReader);

    // Load options
	loadParkingSpots();
		
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
	mServNetHandler->destroy();
	mVideoReader->close();

	// Save current parking spots
	utils::writeJSON(SYSTEM_FOLDER_CORE + SYSTEM_FILE_PARKINGSPOTS, getJSONParkingSpots());
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
		if (mManagementMode) {
			// Do some modification on settings
		}
		else {
			// Draw current status
			output = drawParkingStatus(frame);

			// do something
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

			if (inputKey >= '1' && inputKey <= mParkingSpots.size() + '0') {
				// Add a timer for testing purpose
				int idx = inputKey - '1';
				if (mParkingSpots[idx]->isOccupied()) {
					mParkingSpots[idx]->exit(frame, now);
				}
				else {
					std::cout << "Timer " << std::to_string(idx) << " begins" << std::endl;
					mParkingSpots[idx]->enter(frame, now);
				}
			}
			else if (inputKey == 's' || inputKey == 'S') {
				std::unique_ptr<IMessageData> sync_data = std::make_unique<ServerSyncMessage>(mVideoReader->size(),
					boost::posix_time::second_clock::local_time(), getJSONParkingSpots());
				mServMsgQueue->push(sync_data);
			}
		}
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
		for (const auto &spot : mParkingSpots) {
			rectangle(clonedFrame, spot->ROI, CV_RGB(255, 0, 0));
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

			mParkingSpots.push_back(std::shared_ptr<ParkingSpot>(new ParkingSpot(id, "spot" + std::to_string(id), timeLimit, roi, POLICY_TIMED)));

			break;

		case 'e':	// Remove all parking spots.
		case 'E':
			mParkingSpots.clear();

			break;

		case 'b':	// Remove the most recently added parking spot
		case 'B':
			mParkingSpots.pop_back();

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

	if (mParkingSpots.size() >= 1) {
		if (mSettings->MotionDetectionEnabled) {
			//detectMotion(frame);
		}

		for (auto &parkingSpot : mParkingSpots) {
			vector<Rect> locs;

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

	for (auto &parkingSpot : mParkingSpots) {
		cv::Scalar color;
		if (parkingSpot->isOccupied()) {
			color = CV_RGB(255, 0, 0);
		}
		else {
			color = CV_RGB(0, 255, 0);
		}

		cv::rectangle(drawn, parkingSpot->ROI, color, 2);
	}

	return drawn;
}

void MainInterface::loadParkingSpots() {
	boost::property_tree::ptree ptree;

	boost::property_tree::read_json(SYSTEM_FOLDER_CORE + SYSTEM_FILE_PARKINGSPOTS, ptree);

	for (auto &elem : ptree) {
		int id, timeLimit;
		std::string spotName;
		PARKING_SPOT_POLICY policy;
		cv::Rect roi;

		id = elem.second.get<int>("parkingSpotId");
		spotName = elem.second.get<std::string>("parkingSpotName");
		roi.x = elem.second.get<int>("roiCoordX");
		roi.y = elem.second.get<int>("roiCoordY");
		roi.width = elem.second.get<int>("roiWidth");
		roi.height = elem.second.get<int>("roiHeight");
		policy = (PARKING_SPOT_POLICY)elem.second.get<int>("policyId");
		timeLimit = elem.second.get<int>("timeLimit");

		mParkingSpots.push_back(std::shared_ptr<ParkingSpot>(new ParkingSpot(id, spotName, timeLimit, roi, policy)));
	}
}

boost::property_tree::ptree MainInterface::getJSONParkingSpots() {
	boost::property_tree::ptree spotArray;

	for (auto spot : mParkingSpots) {
		boost::property_tree::ptree elem;

		elem.put("parkingSpotId", std::to_string(spot->ID));
		elem.put("parkingSpotName", spot->SpotName);
		elem.put("roiCoordX", spot->ROI.x);
		elem.put("roiCoordY", spot->ROI.y);
		elem.put("roiWidth", spot->ROI.width);
		elem.put("roiHeight", spot->ROI.height);
		elem.put("policyId", (int)spot->ParkingPolicy);
		elem.put("timeLimit", spot->TimeLimit);

		spotArray.push_back(std::make_pair("", elem));
	}

	return spotArray;
}

void MainInterface::print_usage_roi_settings()
{
	std::cout << "Usage:\n" <<
		"(r) run algorithm on given ROIs, (a) add drawn rectangle as a parking spot,\n"
		"(e) remove all parking spots, (b) remove recently added parking spot,\n" <<
		"(ESC) close the initialization session and run the program" << std::endl;
}
