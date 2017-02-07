#include "main.h"

#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include "ParkingSpot.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    String demoWindowName = "Demo";
    VideoCapture videoCapture;
    Mat frame;
    int inputKey = 0;

    if (!videoCapture.open(0)) {
		cout << "Failed to connect the camera" << endl;
		return 0;
	}

    namedWindow(demoWindowName);

    vector<ParkingSpot> parkingSpots;

    for (int i = 0; i < 9; i++) {
        parkingSpots.push_back(ParkingSpot(10));
    }

    while (videoCapture.read(frame)) {
        imshow(demoWindowName, frame);
        inputKey = 0xFF & waitKey(30);

        if (inputKey == 27) {
            break;
        }

        if (inputKey == 'a') {
            // Add a timer for testing purpose
        }
    }

    destroyAllWindows();

    if (videoCapture.isOpened()) {
		videoCapture.release();
	}
}