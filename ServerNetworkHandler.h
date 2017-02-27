#pragma once

#include "MessageQueue.h"

#include <iostream>

#include <boost/thread.hpp>

#include "ParkingStatus.h"

namespace seevider {
    class ServerNetworkHandler {
    public:
        /**
        * Construct the network handler with a message queue
        */
        ServerNetworkHandler(MessageQueue<ParkingStatus> *messageQueue);

        /**
        * Destruct the network handler
        */
        ~ServerNetworkHandler();

        /**
        * Destroy the network thread
        */
        void destroy();

    private:
        /**
        * Network handling thread
        */
        boost::thread mHandlerThread;

        /**
        * The pointer of message queue
        */
        MessageQueue<ParkingStatus> *mMessageQueue = NULL;

        /**
        * True if the system is running
        */
        bool mOperation = true;

        /**
        * Set it for how long we will wait until the remaining job processes.
        * The unit of this number is second.
        */
        int mWaitSeconds = 2;

        /**
        * Set it for how long we will wait until the handler is destroyed.
        * The unit of this number is second.
        */
        int mDestroySeconds = 3;

        /**
        * Main entry of the handling thread
        */
        void run();

        /**
        * Retrive data from the network message queue, then upload it to the server
        */
        bool upload(const ParkingStatus &data) const;
    };
}