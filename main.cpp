#include "main.h"

#include <string>
#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

#include "IOUtils.h"
#include "MainInterface.h"

using namespace std;
using namespace cv;
using namespace seevider;

int main(int argc, char** argv) {
    MainInterface mainInterface;

    mainInterface.run();

    /*
    // TODO: remove timers for each parking ParkingSpot
    // Instead, when a vehicle enters to a spot,
    // record expiration time to the time list.
    // When the vehicle exit from the spot, remove the expiration time from the list.
    // For each iteration, check if current time equals to one of the expiration times in the list.

    while (videoReader.read(frame)) {
        // Retrieve current local time
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        boost::posix_time::time_duration td = now.time_of_day();
        std::stringstream ss;
        ss << static_cast<int>(now.date().month()) << "/" << now.date().day()
            << "/" << now.date().year() << ", " << td.hours() << ":" << td.minutes() << ":" << td.seconds();
        std::cout << ss.str() << std::endl;
    }*/

}
