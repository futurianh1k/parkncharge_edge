// File:	INetworkHandler.h
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

#include <boost/thread.hpp>

namespace seevider {
	/**
	 * Interface class of a network handler. It provides the function interfaces
	 * necessary to be implemented for the network communication.
	 */
	class INetworkHandler {
	public:
		INetworkHandler();
		~INetworkHandler();

		/**
		 * Destroy the network thread
		 */
		virtual void destroy() = 0;

	protected:
		/**
		 * Network handling thread
		 */
		boost::thread mHandlerThread;

		/**
		 * True if the handler is active
		 */
		bool mOperation;

		/**
		 * Set it for how long we will wait until the remaining job processes.
		 * The unit of this number is second.
		 */
		int mWaitSeconds;

		/**
		 * Set it for how long we will wait until the handler is destroyed.
		 * The unit of this number is second.
		 */
		int mDestroySeconds;

		/**
		 * Main entry of the handling thread
		 */
		virtual void run() = 0;
	private:
	};
}
