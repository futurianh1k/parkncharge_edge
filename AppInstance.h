#pragma once

#include "NetworkHandler.h"

namespace seevider {
    struct AppInstance {
        /**
         * Handling network connection with the server
         */
        NetworkHandler<std::string> networkHandler(msgQueue);
    };
}