// File:	MessageQueue.h
// Author:	Seongdo Kim
// Contact:	sdland85@gmail.com
//
// Copyright (c) 2017, Seongdo Kim <sdland85@gmail.com>
// All rights reserved.
// The copyright to the computer program(s) herein is
// the property of Seongdo Kim. The program(s) may be
// used and/or copied only with the written permission
// of Seongdo Kim or in accordance with the terms and
// conditions stipulated in the agreement/contract under
// which the program(s) have been supplied.
//
// Written by Seongdo Kim <sdland85@gmail.com>, June, 2017

#pragma once

#include <string>
#include <sstream>
#include <queue>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include "IMessageData.h"

namespace seevider {
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
		void push(std::unique_ptr<IMessageData> &data);

		/**
		 * Try to get data from the concurrent queue
		 */
		bool try_pop(std::unique_ptr<IMessageData> &data);

		/**
		 * Try to get data from the concurrent queue.
		 * If no data availaboe, it will wait until a data gets available.
		 */
		void wait_and_pop(std::unique_ptr<IMessageData> &data);

		/**
		 * Remove one data from the front of the queue.
		 */
		void pop();
		
		/**
		 * Construct a string shows queue contents
		 */
		std::string toString() const;

		/**
		 * Check if the concurrent queue is empty
		 */
		bool empty() const;

	private:
		/**
		 * The queue
		 */
		std::queue<std::unique_ptr<IMessageData>> mQueue;

		/**
		 * The mutex to limit the simultaneous access to the queue.
		 */
		mutable boost::mutex mMutex;

		/**
		 * The maximum size of the event queue.
		 */
		int mMaxSize = 10;

		/**
		 * The number of stored data
		 */
		int mNumStoredData = 0;

		/**
		 * The folder name to store overflowed data
		 */
		std::string mDataStorageFolder = "./MessageData/";

		/**
		 * The conditional variable
		 */
		boost::condition_variable mConditionVariable;

		/**
		 * Load stored data and push it to the queue
		 */
		inline void loadStoredData();
	};
}
