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
```
