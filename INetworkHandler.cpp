// File:	INetworkHandler.cpp
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

#include "INetworkHandler.h"

namespace seevider {
	INetworkHandler::INetworkHandler() : mOperation(true), mWaitSeconds(2), mDestroySeconds(3) {
	}

	INetworkHandler::~INetworkHandler(){
	}
}