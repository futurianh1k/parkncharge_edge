#pragma once

namespace seevider {
    // Parking spot status
    enum PARKING_SPOT_STATUS {
        PARKING_SPOT_STATUS_EMPTY,
        PARKING_SPOT_STATUS_OCCUPIED,
        PARKING_SPOT_STATUS_UNAVAILABLE,
        PARKING_SPOT_STATUS_USER
    };

    // Parking spot update status
    enum PARKING_SPOT_UPDATE {
        PARKING_SPOT_UPDATE_ENTER,
        PARKING_SPOT_UPDATE_EXIT,
        PARKING_SPOT_UPDATE_EXPIRED,
        PARKING_SPOT_UPDATE_USER
    };
}