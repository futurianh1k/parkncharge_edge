cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 14)
project(VisionSensor_onlyTF)

#set(Darknet_LIB libdarknet.so)
#link_directories(${PROJECT_SOURCE_DIR}/darknet)
link_directories(/usr/lib/python3.6/config)

find_package(OpenSSL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
option(DEBUG "Select ON to define DEBUG" ON)
message("-- DEBUG = ${DEBUG}")
configure_file(
        "${PROJECT_SOURCE_DIR}/Config.h.in"
        "${PROJECT_BINARY_DIR}/Config.h"
)
set(TARGET_CPP BoardController.cpp CameraInfo.cpp IMessageData.cpp INetworkHandler.cpp IOUtils.cpp MessageQueue.cpp ParkingSpot.cpp ParkingSpotManager.cpp ParkingUpdateMessage.cpp SecureClient.cpp SensorInfo.cpp SerialVideoReader.cpp ServerNetworkHandler.cpp ServerSyncMessage.cpp Settings.cpp TCPSocketListener.cpp Utils.cpp)
add_executable(VisionSensor main.cpp MainInterface.cpp ${TARGET_CPP})
set(BOOST_LIB_LIST -lboost_system -lboost_thread -lboost_chrono -lboost_date_time -lboost_filesystem -lmysqlclient)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(/usr/include/glog)
include_directories(/usr/include/python3.6)
target_link_libraries(VisionSensor ${OPENSSL_LIBRARIES} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${BOOST_LIB_LIST} -lpthread -lglog -lpython3.6m)
#target_link_libraries(VisionSensor ${Darknet_LIB})
