cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 14)
project(VisionSensor)

set (Darknet_LIB libdarknet.so)
link_directories(${PROJECT_SOURCE_DIR}/darknet)

find_package(OpenSSL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
option(DEBUG "Select ON to define DEBUG" ON)
message("-- DEBUG = ${DEBUG}")

configure_file(
        "${PROJECT_SOURCE_DIR}/Config.h.in"
        "${PROJECT_BINARY_DIR}/Config.h"
)
set(TARGET_CPP BoardController.cpp CameraInfo.cpp CascadeClassifier.cpp ImageNetClassifier.cpp IMessageData.cpp INetworkHandler.cpp IGenericDetector.cpp IOccupancyDetector.cpp IPlateDetector.cpp IOUtils.cpp Light.cpp LPR.cpp MessageQueue.cpp MotionDetection.cpp ParkingSpot.cpp ParkingSpotManager.cpp ParkingUpdateMessage.cpp SecureClient.cpp SensorInfo.cpp SerialVideoReader.cpp ServerNetworkHandler.cpp ServerSyncMessage.cpp Settings.cpp TCPSocketListener.cpp Utils.cpp ImageNetClassifier.cpp ImageNetClassifier.h)
add_executable(VisionSensor main.cpp MainInterface.cpp ${TARGET_CPP})
set(BOOST_LIB_LIST -lboost_system -lboost_thread -lboost_chrono -lboost_date_time -lboost_filesystem)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(/usr/include/glog)
target_link_libraries(VisionSensor ${OPENSSL_LIBRARIES} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${BOOST_LIB_LIST} -lpthread -lglog -lopenalpr)
target_link_libraries(VisionSensor ${Darknet_LIB})
