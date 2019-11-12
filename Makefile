# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /media/qisens/4tb1/JH/juhee/11-07/VisionSensor_3model

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /media/qisens/4tb1/JH/juhee/11-07/VisionSensor_3model

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /media/qisens/4tb1/JH/juhee/11-07/VisionSensor_3model/CMakeFiles /media/qisens/4tb1/JH/juhee/11-07/VisionSensor_3model/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /media/qisens/4tb1/JH/juhee/11-07/VisionSensor_3model/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named VisionSensor

# Build rule for target.
VisionSensor: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 VisionSensor
.PHONY : VisionSensor

# fast build rule for target.
VisionSensor/fast:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/build
.PHONY : VisionSensor/fast

BoardController.o: BoardController.cpp.o

.PHONY : BoardController.o

# target to build an object file
BoardController.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/BoardController.cpp.o
.PHONY : BoardController.cpp.o

BoardController.i: BoardController.cpp.i

.PHONY : BoardController.i

# target to preprocess a source file
BoardController.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/BoardController.cpp.i
.PHONY : BoardController.cpp.i

BoardController.s: BoardController.cpp.s

.PHONY : BoardController.s

# target to generate assembly for a file
BoardController.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/BoardController.cpp.s
.PHONY : BoardController.cpp.s

CameraInfo.o: CameraInfo.cpp.o

.PHONY : CameraInfo.o

# target to build an object file
CameraInfo.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/CameraInfo.cpp.o
.PHONY : CameraInfo.cpp.o

CameraInfo.i: CameraInfo.cpp.i

.PHONY : CameraInfo.i

# target to preprocess a source file
CameraInfo.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/CameraInfo.cpp.i
.PHONY : CameraInfo.cpp.i

CameraInfo.s: CameraInfo.cpp.s

.PHONY : CameraInfo.s

# target to generate assembly for a file
CameraInfo.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/CameraInfo.cpp.s
.PHONY : CameraInfo.cpp.s

IMessageData.o: IMessageData.cpp.o

.PHONY : IMessageData.o

# target to build an object file
IMessageData.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/IMessageData.cpp.o
.PHONY : IMessageData.cpp.o

IMessageData.i: IMessageData.cpp.i

.PHONY : IMessageData.i

# target to preprocess a source file
IMessageData.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/IMessageData.cpp.i
.PHONY : IMessageData.cpp.i

IMessageData.s: IMessageData.cpp.s

.PHONY : IMessageData.s

# target to generate assembly for a file
IMessageData.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/IMessageData.cpp.s
.PHONY : IMessageData.cpp.s

INetworkHandler.o: INetworkHandler.cpp.o

.PHONY : INetworkHandler.o

# target to build an object file
INetworkHandler.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/INetworkHandler.cpp.o
.PHONY : INetworkHandler.cpp.o

INetworkHandler.i: INetworkHandler.cpp.i

.PHONY : INetworkHandler.i

# target to preprocess a source file
INetworkHandler.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/INetworkHandler.cpp.i
.PHONY : INetworkHandler.cpp.i

INetworkHandler.s: INetworkHandler.cpp.s

.PHONY : INetworkHandler.s

# target to generate assembly for a file
INetworkHandler.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/INetworkHandler.cpp.s
.PHONY : INetworkHandler.cpp.s

IOUtils.o: IOUtils.cpp.o

.PHONY : IOUtils.o

# target to build an object file
IOUtils.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/IOUtils.cpp.o
.PHONY : IOUtils.cpp.o

IOUtils.i: IOUtils.cpp.i

.PHONY : IOUtils.i

# target to preprocess a source file
IOUtils.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/IOUtils.cpp.i
.PHONY : IOUtils.cpp.i

IOUtils.s: IOUtils.cpp.s

.PHONY : IOUtils.s

# target to generate assembly for a file
IOUtils.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/IOUtils.cpp.s
.PHONY : IOUtils.cpp.s

MainInterface.o: MainInterface.cpp.o

.PHONY : MainInterface.o

# target to build an object file
MainInterface.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/MainInterface.cpp.o
.PHONY : MainInterface.cpp.o

MainInterface.i: MainInterface.cpp.i

.PHONY : MainInterface.i

# target to preprocess a source file
MainInterface.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/MainInterface.cpp.i
.PHONY : MainInterface.cpp.i

MainInterface.s: MainInterface.cpp.s

.PHONY : MainInterface.s

# target to generate assembly for a file
MainInterface.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/MainInterface.cpp.s
.PHONY : MainInterface.cpp.s

MessageQueue.o: MessageQueue.cpp.o

.PHONY : MessageQueue.o

# target to build an object file
MessageQueue.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/MessageQueue.cpp.o
.PHONY : MessageQueue.cpp.o

MessageQueue.i: MessageQueue.cpp.i

.PHONY : MessageQueue.i

# target to preprocess a source file
MessageQueue.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/MessageQueue.cpp.i
.PHONY : MessageQueue.cpp.i

MessageQueue.s: MessageQueue.cpp.s

.PHONY : MessageQueue.s

# target to generate assembly for a file
MessageQueue.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/MessageQueue.cpp.s
.PHONY : MessageQueue.cpp.s

ParkingSpot.o: ParkingSpot.cpp.o

.PHONY : ParkingSpot.o

# target to build an object file
ParkingSpot.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingSpot.cpp.o
.PHONY : ParkingSpot.cpp.o

ParkingSpot.i: ParkingSpot.cpp.i

.PHONY : ParkingSpot.i

# target to preprocess a source file
ParkingSpot.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingSpot.cpp.i
.PHONY : ParkingSpot.cpp.i

ParkingSpot.s: ParkingSpot.cpp.s

.PHONY : ParkingSpot.s

# target to generate assembly for a file
ParkingSpot.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingSpot.cpp.s
.PHONY : ParkingSpot.cpp.s

ParkingSpotManager.o: ParkingSpotManager.cpp.o

.PHONY : ParkingSpotManager.o

# target to build an object file
ParkingSpotManager.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingSpotManager.cpp.o
.PHONY : ParkingSpotManager.cpp.o

ParkingSpotManager.i: ParkingSpotManager.cpp.i

.PHONY : ParkingSpotManager.i

# target to preprocess a source file
ParkingSpotManager.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingSpotManager.cpp.i
.PHONY : ParkingSpotManager.cpp.i

ParkingSpotManager.s: ParkingSpotManager.cpp.s

.PHONY : ParkingSpotManager.s

# target to generate assembly for a file
ParkingSpotManager.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingSpotManager.cpp.s
.PHONY : ParkingSpotManager.cpp.s

ParkingUpdateMessage.o: ParkingUpdateMessage.cpp.o

.PHONY : ParkingUpdateMessage.o

# target to build an object file
ParkingUpdateMessage.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingUpdateMessage.cpp.o
.PHONY : ParkingUpdateMessage.cpp.o

ParkingUpdateMessage.i: ParkingUpdateMessage.cpp.i

.PHONY : ParkingUpdateMessage.i

# target to preprocess a source file
ParkingUpdateMessage.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingUpdateMessage.cpp.i
.PHONY : ParkingUpdateMessage.cpp.i

ParkingUpdateMessage.s: ParkingUpdateMessage.cpp.s

.PHONY : ParkingUpdateMessage.s

# target to generate assembly for a file
ParkingUpdateMessage.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ParkingUpdateMessage.cpp.s
.PHONY : ParkingUpdateMessage.cpp.s

SecureClient.o: SecureClient.cpp.o

.PHONY : SecureClient.o

# target to build an object file
SecureClient.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SecureClient.cpp.o
.PHONY : SecureClient.cpp.o

SecureClient.i: SecureClient.cpp.i

.PHONY : SecureClient.i

# target to preprocess a source file
SecureClient.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SecureClient.cpp.i
.PHONY : SecureClient.cpp.i

SecureClient.s: SecureClient.cpp.s

.PHONY : SecureClient.s

# target to generate assembly for a file
SecureClient.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SecureClient.cpp.s
.PHONY : SecureClient.cpp.s

SensorInfo.o: SensorInfo.cpp.o

.PHONY : SensorInfo.o

# target to build an object file
SensorInfo.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SensorInfo.cpp.o
.PHONY : SensorInfo.cpp.o

SensorInfo.i: SensorInfo.cpp.i

.PHONY : SensorInfo.i

# target to preprocess a source file
SensorInfo.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SensorInfo.cpp.i
.PHONY : SensorInfo.cpp.i

SensorInfo.s: SensorInfo.cpp.s

.PHONY : SensorInfo.s

# target to generate assembly for a file
SensorInfo.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SensorInfo.cpp.s
.PHONY : SensorInfo.cpp.s

SerialVideoReader.o: SerialVideoReader.cpp.o

.PHONY : SerialVideoReader.o

# target to build an object file
SerialVideoReader.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SerialVideoReader.cpp.o
.PHONY : SerialVideoReader.cpp.o

SerialVideoReader.i: SerialVideoReader.cpp.i

.PHONY : SerialVideoReader.i

# target to preprocess a source file
SerialVideoReader.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SerialVideoReader.cpp.i
.PHONY : SerialVideoReader.cpp.i

SerialVideoReader.s: SerialVideoReader.cpp.s

.PHONY : SerialVideoReader.s

# target to generate assembly for a file
SerialVideoReader.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/SerialVideoReader.cpp.s
.PHONY : SerialVideoReader.cpp.s

ServerNetworkHandler.o: ServerNetworkHandler.cpp.o

.PHONY : ServerNetworkHandler.o

# target to build an object file
ServerNetworkHandler.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ServerNetworkHandler.cpp.o
.PHONY : ServerNetworkHandler.cpp.o

ServerNetworkHandler.i: ServerNetworkHandler.cpp.i

.PHONY : ServerNetworkHandler.i

# target to preprocess a source file
ServerNetworkHandler.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ServerNetworkHandler.cpp.i
.PHONY : ServerNetworkHandler.cpp.i

ServerNetworkHandler.s: ServerNetworkHandler.cpp.s

.PHONY : ServerNetworkHandler.s

# target to generate assembly for a file
ServerNetworkHandler.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ServerNetworkHandler.cpp.s
.PHONY : ServerNetworkHandler.cpp.s

ServerSyncMessage.o: ServerSyncMessage.cpp.o

.PHONY : ServerSyncMessage.o

# target to build an object file
ServerSyncMessage.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ServerSyncMessage.cpp.o
.PHONY : ServerSyncMessage.cpp.o

ServerSyncMessage.i: ServerSyncMessage.cpp.i

.PHONY : ServerSyncMessage.i

# target to preprocess a source file
ServerSyncMessage.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ServerSyncMessage.cpp.i
.PHONY : ServerSyncMessage.cpp.i

ServerSyncMessage.s: ServerSyncMessage.cpp.s

.PHONY : ServerSyncMessage.s

# target to generate assembly for a file
ServerSyncMessage.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/ServerSyncMessage.cpp.s
.PHONY : ServerSyncMessage.cpp.s

Settings.o: Settings.cpp.o

.PHONY : Settings.o

# target to build an object file
Settings.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/Settings.cpp.o
.PHONY : Settings.cpp.o

Settings.i: Settings.cpp.i

.PHONY : Settings.i

# target to preprocess a source file
Settings.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/Settings.cpp.i
.PHONY : Settings.cpp.i

Settings.s: Settings.cpp.s

.PHONY : Settings.s

# target to generate assembly for a file
Settings.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/Settings.cpp.s
.PHONY : Settings.cpp.s

TCPSocketListener.o: TCPSocketListener.cpp.o

.PHONY : TCPSocketListener.o

# target to build an object file
TCPSocketListener.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/TCPSocketListener.cpp.o
.PHONY : TCPSocketListener.cpp.o

TCPSocketListener.i: TCPSocketListener.cpp.i

.PHONY : TCPSocketListener.i

# target to preprocess a source file
TCPSocketListener.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/TCPSocketListener.cpp.i
.PHONY : TCPSocketListener.cpp.i

TCPSocketListener.s: TCPSocketListener.cpp.s

.PHONY : TCPSocketListener.s

# target to generate assembly for a file
TCPSocketListener.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/TCPSocketListener.cpp.s
.PHONY : TCPSocketListener.cpp.s

Utils.o: Utils.cpp.o

.PHONY : Utils.o

# target to build an object file
Utils.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/Utils.cpp.o
.PHONY : Utils.cpp.o

Utils.i: Utils.cpp.i

.PHONY : Utils.i

# target to preprocess a source file
Utils.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/Utils.cpp.i
.PHONY : Utils.cpp.i

Utils.s: Utils.cpp.s

.PHONY : Utils.s

# target to generate assembly for a file
Utils.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/Utils.cpp.s
.PHONY : Utils.cpp.s

main.o: main.cpp.o

.PHONY : main.o

# target to build an object file
main.cpp.o:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/main.cpp.o
.PHONY : main.cpp.o

main.i: main.cpp.i

.PHONY : main.i

# target to preprocess a source file
main.cpp.i:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/main.cpp.i
.PHONY : main.cpp.i

main.s: main.cpp.s

.PHONY : main.s

# target to generate assembly for a file
main.cpp.s:
	$(MAKE) -f CMakeFiles/VisionSensor.dir/build.make CMakeFiles/VisionSensor.dir/main.cpp.s
.PHONY : main.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... VisionSensor"
	@echo "... edit_cache"
	@echo "... BoardController.o"
	@echo "... BoardController.i"
	@echo "... BoardController.s"
	@echo "... CameraInfo.o"
	@echo "... CameraInfo.i"
	@echo "... CameraInfo.s"
	@echo "... IMessageData.o"
	@echo "... IMessageData.i"
	@echo "... IMessageData.s"
	@echo "... INetworkHandler.o"
	@echo "... INetworkHandler.i"
	@echo "... INetworkHandler.s"
	@echo "... IOUtils.o"
	@echo "... IOUtils.i"
	@echo "... IOUtils.s"
	@echo "... MainInterface.o"
	@echo "... MainInterface.i"
	@echo "... MainInterface.s"
	@echo "... MessageQueue.o"
	@echo "... MessageQueue.i"
	@echo "... MessageQueue.s"
	@echo "... ParkingSpot.o"
	@echo "... ParkingSpot.i"
	@echo "... ParkingSpot.s"
	@echo "... ParkingSpotManager.o"
	@echo "... ParkingSpotManager.i"
	@echo "... ParkingSpotManager.s"
	@echo "... ParkingUpdateMessage.o"
	@echo "... ParkingUpdateMessage.i"
	@echo "... ParkingUpdateMessage.s"
	@echo "... SecureClient.o"
	@echo "... SecureClient.i"
	@echo "... SecureClient.s"
	@echo "... SensorInfo.o"
	@echo "... SensorInfo.i"
	@echo "... SensorInfo.s"
	@echo "... SerialVideoReader.o"
	@echo "... SerialVideoReader.i"
	@echo "... SerialVideoReader.s"
	@echo "... ServerNetworkHandler.o"
	@echo "... ServerNetworkHandler.i"
	@echo "... ServerNetworkHandler.s"
	@echo "... ServerSyncMessage.o"
	@echo "... ServerSyncMessage.i"
	@echo "... ServerSyncMessage.s"
	@echo "... Settings.o"
	@echo "... Settings.i"
	@echo "... Settings.s"
	@echo "... TCPSocketListener.o"
	@echo "... TCPSocketListener.i"
	@echo "... TCPSocketListener.s"
	@echo "... Utils.o"
	@echo "... Utils.i"
	@echo "... Utils.s"
	@echo "... main.o"
	@echo "... main.i"
	@echo "... main.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

