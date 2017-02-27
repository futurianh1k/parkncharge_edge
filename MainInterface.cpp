#include "MainInterface.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

using namespace seevider;
namespace pt = boost::posix_time;

using cv::Mat;

MainInterface::MainInterface() :
mOperation(true) {
    // Construct core resource instances
    mVideoReader = new SerialVideoReader(0);
    mServMsgQueue = new MessageQueue<ParkingStatus>();

    // Start the resource managing threads.
    mServNetHandler = new ServerNetworkHandler(mServMsgQueue);

    // Share message queues
    ParkingSpot::setMessageQueue(mServMsgQueue);
    ParkingSpot::setVideoReader(mVideoReader);

    // Load options

    // Load data -- debuging
    for (int i = 0; i < 10; i++) {
        mParkingSpots.push_back(boost::shared_ptr<ParkingSpot>(new ParkingSpot(std::to_string(i), 10)));
    }
}

MainInterface::~MainInterface() {
    // Destroy running threads
    delete mServNetHandler;

    // Release resources
    delete mServMsgQueue;
    delete mVideoReader;
}

void MainInterface::run() {
    int inputKey = 0;
    Mat frame;
    pt::ptime now;

    cv::namedWindow(mDebugWindowName);

    while (mOperation && mVideoReader->read(frame, now)) {
        // do something
        cv::imshow(mDebugWindowName, frame);
        inputKey = 0xFF & cv::waitKey(30);

        if (inputKey == 27) {
            break;
        }

        if (inputKey >= '0' && inputKey <= '9') {
            // Add a timer for testing purpose
            int idx = inputKey - '0';
            if (mParkingSpots[idx]->isOccupied()) {
                mParkingSpots[idx]->exit(frame, now);
            }
            else {
                std::cout << "Timer " << std::to_string(idx) << " begins" << std::endl;
                mParkingSpots[idx]->enter(frame, now);
            }
        }
    }

    cv::destroyWindow(mDebugWindowName);
    std::cout << "Finish the main activity" << std::endl;

    // Release resources
    mServNetHandler->destroy();
}
