//
// Created by son on 18. 4. 25.
//

#ifndef VISIONSENSOR_IMAGENETCLASSIFIER_H
#define VISIONSENSOR_IMAGENETCLASSIFIER_H

#include <boost/asio/detail/shared_ptr.hpp>
#include "Settings.h"
#include "IGenericDetector.h"

extern "C"
{
#include "darknet/include/darknet.h"
//#include "darknet/src/network.h"
};

namespace seevider {
    class ImageNetClassifier :
            public IGenericDetector{
    public:
        ImageNetClassifier(std::shared_ptr<Settings> &settings);
        ~ImageNetClassifier();
        void ipl_into_image(IplImage* src, image im);
        image ipl_to_image(IplImage* src);
        std::vector<char> str_to_cstr(std::string str);
        virtual int detect(const cv::Mat& img, std::vector<cv::Rect> &locs, int size);
    private:
        std::shared_ptr<Settings> mSettings = nullptr;
        network *net;
    };
};


#endif //VISIONSENSOR_IMAGENETCLASSIFIER_H
