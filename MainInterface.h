#pragma once

#include "SerialVideoReader.h"
#include "ServerNetworkHandler.h"
#include "ParkingSpot.h"

#include <boost/thread.hpp>

namespace seevider {
    /**
     * Main interface class. An instance of the class must be run within the main thread.
     * Otherwise, the application will be crashed due to the OpenCV windows.
     * The purpose of having this class is to manage various resources easily.
     */
    class MainInterface {
    public:
        MainInterface();
        ~MainInterface();

        /**
        * Main entry of the class
        */
        void run();

    private:
        /**
         * Must be true while the system is operating.
         */
        bool mOperation;

        /**
         * Pointer of the video reader.
         */
        SerialVideoReader *mVideoReader;

        /**
         * The massage queue for the server communication
         */
        MessageQueue<ParkingStatus> *mServMsgQueue;

        /**
         * The server-side network handler
         */
        ServerNetworkHandler *mServNetHandler;

        /**
         * An array of parking spots
         */
        std::vector<boost::shared_ptr<ParkingSpot>> mParkingSpots;

        /**
         * Window name for debugging
         */
        cv::String mDebugWindowName = "Debugging Camera View";
    };
}
