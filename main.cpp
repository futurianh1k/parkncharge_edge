#include "main.h"

#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>

#include "ParkingSpot.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    String demoWindowName = "Demo";
    VideoCapture videoCapture;
    Mat frame;
    int inputKey = 0;

    //boost::asio::io_service io;
    //boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));

    if (!videoCapture.open(0)) {
		cout << "Failed to connect the camera" << endl;
		return 0;
	}

    namedWindow(demoWindowName);

    vector<boost::shared_ptr<ParkingSpot>> parkingSpots;

    for (int i = 0; i < 10; i++) {
        parkingSpots.push_back(boost::shared_ptr<ParkingSpot>(new ParkingSpot(std::to_string(i), 10)));
    }

    while (videoCapture.read(frame)) {
        imshow(demoWindowName, frame);
        inputKey = 0xFF & waitKey(30);

        if (inputKey == 27) {
            break;
        }

        if (inputKey >= '0' && inputKey <= '9') {
            // Add a timer for testing purpose
            int idx = inputKey - '0';
            if (parkingSpots[idx]->isOccupied()) {
                parkingSpots[idx]->exit(cv::Mat());
            }
            else {
                cout << "Timer " << to_string(idx) << " begins" << endl;
                parkingSpots[idx]->enter(cv::Mat());
            }
        }
    }

    destroyAllWindows();

    if (videoCapture.isOpened()) {
		videoCapture.release();
	}
}