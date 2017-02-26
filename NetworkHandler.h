#pragma once

#include "MessageQueue.h"

#include <iostream>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include "ParkingStatus.h"

template<typename Data>
class NetworkHandler {
public:
    /**
     * Construct the network handler with a message queue
     */
    NetworkHandler(MessageQueue<Data> *messageQueue);

    /**
     * Destruct the network handler
     */
    ~NetworkHandler();

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
    MessageQueue<Data> *mMessageQueue = NULL;

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
    bool upload(const Data &data) const;
};

template<typename Data>
NetworkHandler<Data>::NetworkHandler(MessageQueue<Data> *messageQueue) : 
mHandlerThread(boost::bind(&NetworkHandler::run, this)) {
    assert(messageQueue != NULL);
    mMessageQueue = messageQueue;
}

template<typename Data>
NetworkHandler<Data>::~NetworkHandler() {
    if (mHandlerThread.joinable()) {
        // If the thread is still joinable, destroy it.
        destroy();
    }
}

template<typename Data>
void NetworkHandler<Data>::destroy() {
    std::cout << "Destroy network handler...";
    mOperation = false;
    mHandlerThread.try_join_for(boost::chrono::seconds(mWaitSeconds));
    if (mHandlerThread.joinable()) {
        // If the thread is still joinable, force to cancel it.
        mHandlerThread.interrupt();
        mHandlerThread.try_join_for(boost::chrono::seconds(mDestroySeconds));
    }
    std::cout << "done." << std::endl;
}

template<typename Data>
void NetworkHandler<Data>::run() {
    std::cout << "Begin network handler" << std::endl;
    while (mOperation) {
        Data data;
        mMessageQueue->wait_and_pop(data);

        // upload data to destination server in designated format
        if (!upload(data)) {
            // If failed to upload the data, push the data back to the MessageQueue
            mMessageQueue->push(data);
        }
    }
    std::cout << "Finish network handler" << std::endl;
}

template<typename Data>
bool NetworkHandler<Data>::upload(const Data &data) const {
    // TODO: upload the retrieved data
    std::cout << "Processing data: " << data << std::endl;

    return true;
}