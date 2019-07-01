//
// Created by son on 18. 4. 25.
//

#include <log_severity.h>
#include <logging.h>
#include "ImageNetClassifier.h"

namespace seevider {
    ImageNetClassifier::ImageNetClassifier(std::shared_ptr<seevider::Settings> &settings) {
        mSettings = settings;
        std::vector<char> cfgFile = str_to_cstr(mSettings->CfgFile);
        std::vector<char> weightFile = str_to_cstr(mSettings->WeightFile);
        net = load_network(cfgFile.data(), weightFile.data(), 0);
    }

    ImageNetClassifier::~ImageNetClassifier() {}

    std::vector<char> ImageNetClassifier::str_to_cstr(std::string str) {
        std::vector<char> cstr(str.c_str(), str.c_str() + str.size() + 1);
        return cstr;
    }

    void ImageNetClassifier::ipl_into_image(IplImage *src, image im) {
        unsigned char *data = (unsigned char *) src->imageData;
        int h = src->height;
        int w = src->width;
        int c = src->nChannels;
        int step = src->widthStep;
        int i, j, k;

        for (i = 0; i < h; ++i) {
            for (k = 0; k < c; ++k) {
                for (j = 0; j < w; ++j) {
                    im.data[k * w * h + i * w + j] = data[i * step + j * c + k] / 255.;
                }
            }
        }

    }

    image ImageNetClassifier::ipl_to_image(IplImage *src) {
        int h = src->height;
        int w = src->width;
        int c = src->nChannels;
        image out = make_image(w, h, c);
        ipl_into_image(src, out);
        return out;

    }

    int ImageNetClassifier::detect(const cv::Mat &img, std::vector<cv::Rect> &locs, int size) {
        set_batch_network(net, 1);
        srand(2222222);

//        list *options = read_data_cfg(mSettings->Datacfg);
//        char *name_list = option_find_str(options, "labels", "darknet/data/labels.list");

        IplImage *frame = new IplImage(img);

        clock_t time;
        char buff[256];
        char *input = buff;

        while (1) {
            image im = ipl_to_image(frame);
            image r = letterbox_image(im, net->w, net->h);
            float *X = r.data;
            time = clock();
            network orig = *net;
            net->input = X;
            forward_network(net);
            float *out = net->output;
            *net = orig;
            float *predictions = out;

            //fprintf(stderr, "%s: Predicted in %f seconds.\n", input, sec(clock() - time));
            if (r.data != im.data) free_image(r);
            free_image(im);
            //printf("prediction: %f\n", *predictions);
            if (*predictions > 0.8) return true;
            else return false;
        }
    }
}
