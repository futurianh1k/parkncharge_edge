#include "INetworkHandler.h"

namespace seevider {
	INetworkHandler::INetworkHandler() : mOperation(true), mWaitSeconds(2), mDestroySeconds(3) {
	}

	INetworkHandler::~INetworkHandler(){
	}
}