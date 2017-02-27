#include "ServerNetworkHandler.h"

#include <boost/chrono.hpp>

using namespace seevider;

ServerNetworkHandler::ServerNetworkHandler(MessageQueue<ParkingStatus> *messageQueue) : 
mHandlerThread(boost::bind(&ServerNetworkHandler::run, this)) {
    assert(messageQueue != NULL);
    mMessageQueue = messageQueue;
}

ServerNetworkHandler::~ServerNetworkHandler() {
    if (mHandlerThread.joinable()) {
        // If the thread is still joinable, destroy it.
        destroy();
    }
}

void ServerNetworkHandler::destroy() {
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

void ServerNetworkHandler::run() {
    std::cout << "Starts the network handler" << std::endl;
    while (mOperation) {
        ParkingStatus data;
        mMessageQueue->wait_and_pop(data);

        // upload data to destination server in designated format
        if (!upload(data)) {
            // If failed to upload the data, push the data back to the MessageQueue
            mMessageQueue->push(data);
        }
    }
    std::cout << "Ends the network handler" << std::endl;
}

bool ServerNetworkHandler::upload(const ParkingStatus &data) const {
    // TODO: upload the retrieved data
    std::cout << "Processing data: " << data.toString() << std::endl;

    return true;
}
