#pragma once

#include <string>
#include <sstream>
#include <queue>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

template<typename Data>
class MessageQueue {
public:
    /**
     * Basic constructor
     */
    MessageQueue();

    /**
     * Basic destructor
     */
    ~MessageQueue();

    /**
     * Push data into the concurrent queue
     */
    void push(Data const& data);

    /**
     * Try to get data from the concurrent queue
     */
    bool try_pop(Data& popped_value);

    /**
     * Try to get data from the concurrent queue.
     * If no data availaboe, it will wait until a data gets available.
     */
    void wait_and_pop(Data& popped_value);

    /**
     * Construct a string shows queue contents
     */
    std::string toString();

    /**
     * Check if the concurrent queue is empty
     */
    bool empty() const;

private:
    /**
     * The queue
     */
    std::queue<Data> mQueue;

    /**
     * The mutex to limit the simultaneous access to the queue.
     */
    mutable boost::mutex mMutex;

    /**
     * The conditional variable 
     */
    boost::condition_variable mConditionVariable;
};

template<typename Data>
MessageQueue<Data>::MessageQueue() {
}

template<typename Data>
MessageQueue<Data>::~MessageQueue() {
}

template<typename Data>
void MessageQueue<Data>::push(Data const& data) {
    boost::mutex::scoped_lock lock(mMutex);
    mQueue.push(data);
    lock.unlock();
    mConditionVariable.notify_one();
}

template<typename Data>
bool MessageQueue<Data>::try_pop(Data& popped_value) {
    boost::mutex::scoped_lock lock(mMutex);
    if(mQueue.empty()) {
        return false;
    }
    popped_value = mQueue.front();
    mQueue.pop();
    return true;
}

template<typename Data>
void MessageQueue<Data>::wait_and_pop(Data& popped_value) {
    boost::mutex::scoped_lock lock(mMutex);
    while(mQueue.empty()) {
        mConditionVariable.wait(lock);
    }
    popped_value = mQueue.front();
    mQueue.pop();
}

template<typename Data>
bool MessageQueue<Data>::empty() const {
    boost::mutex::scoped_lock lock(mMutex);
    return mQueue.empty();
}

template<typename Data>
std::string MessageQueue<Data>::toString() {
    std::stringstream output;

    boost::mutex::scoped_lock lock(mMutex);

    output << "#Contents: " << mQueue.size();

    return output.str();
}