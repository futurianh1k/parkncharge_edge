#include "MessageQueue.h"
#include "IOUtils.h"

using namespace seevider;

MessageQueue::MessageQueue() {
}

MessageQueue::~MessageQueue() {
}

/**
 * Push data into the concurrent queue
 */
void MessageQueue::push(std::unique_ptr<IMessageData> &data) {
	boost::mutex::scoped_lock lock(mMutex);

	if (mQueue.size() >= mMaxSize) {
		if (data->save(mDataStorageFolder)) {
			mNumStoredData++;
		}
	}
	else {	// memory is enough
		mQueue.push(std::move(data));
	}

	lock.unlock();
	mConditionVariable.notify_one();
}

bool MessageQueue::try_pop(std::unique_ptr<IMessageData> &data) {
	boost::mutex::scoped_lock lock(mMutex);

	loadStoredData();

	if (mQueue.empty()) {
		data = NULL;
		return false;
	}

	data = std::move(mQueue.front());
	mQueue.pop();
	return true;
}

void MessageQueue::wait_and_pop(std::unique_ptr<IMessageData> &data) {
	boost::mutex::scoped_lock lock(mMutex);

	loadStoredData();

	while (mQueue.empty()) {
		mConditionVariable.wait(lock);
	}

	data = std::move(mQueue.front());
	mQueue.pop();
}

void MessageQueue::pop() {
	boost::mutex::scoped_lock lock(mMutex);
	mQueue.pop();
}

bool MessageQueue::empty() const {
	boost::mutex::scoped_lock lock(mMutex);
	return mQueue.empty();
}

std::string MessageQueue::toString() const {
	std::stringstream output;

	boost::mutex::scoped_lock lock(mMutex);

	output << "#Contents: " << mQueue.size();

	return output.str();
}

void MessageQueue::loadStoredData() {
	if (mQueue.size() < mMaxSize && mNumStoredData > 0) {
		std::unique_ptr<IMessageData> storedData;

		if (utils::loadMessage(storedData, mDataStorageFolder)) {
			mQueue.push(std::move(storedData));
			mNumStoredData--;
		}
	}
}