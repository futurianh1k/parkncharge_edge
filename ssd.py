# -*- coding: utf-8 -*-


# Change for Python3
# cmp
# xrange
# SSD BBox class changed : Added label

import caffe
import numpy as np

import geo
import skimage.exposure
from skimage.color import rgb2gray


from google.protobuf import text_format


def cmp(a, b):
    if a>b:
        return 1
    elif a<b:
        return -1
    else:
        return 0

    #return (a > b) - (a < b)


class BBox(object):
    def __init__(self, rect, conf, name, label):
        self.rect = rect
        self.conf = conf
        self.name = name
        self.label = label



    @property
    def center(self):
        return self.rect.center

    @property
    def area(self):
        return self.rect.area

    @classmethod
    def xcmp(cls, bbox1, bbox2):
        return cmp(bbox1.rect.xline.low, bbox2.rect.xline.low)

    @classmethod
    def ycmp(cls, bbox1, bbox2):
        return cmp(bbox1.rect.yline.low, bbox2.rect.yline.low)

    @classmethod
    def confcmp(cls, bbox1, bbox2):
        return cmp(bbox1.conf, bbox2.conf)

class Detection(object):
    def __init__(self, image, bboxes):
        self.image = image
        self.bboxes = bboxes

class SSD(object):
    def __init__(self, model_def, model_weights, labelmap_file, conf_threshold):
        self.model_def = model_def
        self.model_weights = model_weights
        self.labelmap_file = labelmap_file
        self.conf_threshold = conf_threshold

        # Network
        self.net = caffe.Net(self.model_def, self.model_weights, caffe.TEST)
        self.input_name = self.net.inputs[0]
        self.output_name = self.net.outputs[0]

        input_blob = self.net.blobs[self.input_name]
        self.input_channels = input_blob.data.shape[1]
        self.input_height = input_blob.data.shape[2]
        self.input_width = input_blob.data.shape[3]

        input_blob.reshape(1, self.input_channels,
                           self.input_height,
                           self.input_width)

        self._create_transformer()
        self._create_labelmap()

    def _create_transformer(self):
        input_blob = self.net.blobs[self.input_name]
        self.transformer = caffe.io.Transformer({self.input_name: input_blob.data.shape})
        self.transformer.set_transpose(self.input_name, (2, 0, 1)) # HWC -> CHW
        self.transformer.set_mean(self.input_name, np.array([104, 117, 123]))
        self.transformer.set_raw_scale(self.input_name, 255)
        self.transformer.set_channel_swap(self.input_name, (2, 1, 0)) # RGB -> BGR

    def _create_labelmap(self):
        with open(self.labelmap_file, 'r') as f:
            self.labelmap = caffe.proto.caffe_pb2.LabelMap()
            text_format.Merge(str(f.read()), self.labelmap)

    def get_labelname(self, labels):
        labelmap = self.labelmap
        num_labels = len(labelmap.item)
        labelnames = []

        if type(labels) is not list:
            labels = [labels]

        for label in labels:
                found = False
                for i in range(num_labels):
                    if label == labelmap.item[i].label:
                        found = True
                        labelnames.append(labelmap.item[i].display_name)
                        break
                assert found == True
        return labelnames

    def detect_with_gtgen(self, image, anno_list, confidence):#SEAN : Annotation list as an argument

        bboxes = []

        for i in range(len(anno_list)):
            xmin = int(anno_list[i][1])
            ymin = int(anno_list[i][2])
            xmax = int(anno_list[i][3])
            ymax = int(anno_list[i][4])
            conf = 0.99

            label_name = self.labelmap.item[int(anno_list[i][0])].display_name

            xline = geo.Line(xmin, xmax)
            yline = geo.Line(ymin, ymax)
            rect = geo.Rect(xline, yline)
            bbox = BBox(rect, conf, label_name)
            bboxes.append(bbox)

        return Detection(image, bboxes)

    def detect(self, image, confidence):
        transformed = self.transformer.preprocess(self.input_name, image)
        self.net.blobs[self.input_name].data[...] = transformed

        detections = self.net.forward()[self.output_name]
        det_label = detections[0, 0, :, 1]
        det_conf = detections[0, 0, :, 2]
        det_xmin = detections[0, 0, :, 3]
        det_ymin = detections[0, 0, :, 4]
        det_xmax = detections[0, 0, :, 5]
        det_ymax = detections[0, 0, :, 6]

        top_indices = [i for i, conf in enumerate(det_conf) if conf >= confidence]
        #top_indices = [i for i, conf in enumerate(det_conf) if conf >= self.conf_threshold]
        top_conf = det_conf[top_indices]
        top_label_indices = det_label[top_indices].tolist()
        top_labels = self.get_labelname(top_label_indices)
        top_xmin = det_xmin[top_indices]
        top_ymin = det_ymin[top_indices]
        top_xmax = det_xmax[top_indices]
        top_ymax = det_ymax[top_indices]

        bboxes = []
        img_ymax, img_xmax = image.shape[:2]
        for i in range(top_conf.shape[0]):
            xmin = max(int(round(top_xmin[i] * image.shape[1])), 0)
            ymin = max(int(round(top_ymin[i] * image.shape[0])), 0)
            xmax = min(int(round(top_xmax[i] * image.shape[1])), img_xmax)
            ymax = min(int(round(top_ymax[i] * image.shape[0])), img_ymax)
            conf = top_conf[i]
            label = int(top_label_indices[i])
            label_name = top_labels[i]

            xline = geo.Line(xmin, xmax)
            yline = geo.Line(ymin, ymax)
            rect = geo.Rect(xline, yline)
            bbox = BBox(rect, conf, label_name,label)
            bboxes.append(bbox)

        return Detection(image, bboxes)
