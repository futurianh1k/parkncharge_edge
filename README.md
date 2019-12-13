# how to add c++ library
: CMakeLists.txt => target_link_libraries => openssl_libraries => add used library

# Vision Sensor
Vision sensor program.

# Linux Environment Preparation
Run following commands on the Linux shell before compile or run the program.
```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install -y build-essential cmake git pkg-config
sudo apt-get install -y cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install -y libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev v4l-utils
sudo apt-get install -y libatlas-base-dev

# (GLOG)
sudo apt-get install -y libgflags-dev libgoogle-glog-dev

# (OpenCV 2.4)
sudo apt-get install -y libopencv-dev

# (OpenSSL)
sudo apt-get -y install openssl
sudo apt-get -y install libssl-dev

# (Boost)
sudo apt-get install -y --no-install-recommends libboost-all-dev

# (OpenALPR)
## Ubuntu 16.04
sudo apt-get install -y openalpr openalpr-daemon openalpr-utils libopenalpr-dev

## Linux-based OS other than Ubuntu 16.04
sudo apt-get install libtesseract-dev libleptonica-dev liblog4cplus-dev libcurl3-dev
git clone https://github.com/openalpr/openalpr.git

### Setup the build directory
cd openalpr/src
mkdir build
cd build

### setup the compile environment
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_INSTALL_SYSCONFDIR:PATH=/etc ..

### makefile claen
./initCmake.sh

### compile the library
make

### Install the binaries/libraries to your local system (prefix is /usr)
sudo make install
```

*If run with tensorflow, must be copy or clone tflib*
