#!/usr/bin/env python3

import cv2
import numpy as np
import tensorflow as tf
from tflib.object_detection.utils import label_map_util

from PIL import ImageFont, ImageDraw, Image
# NUM_CLASSES = 88
# MIN_THRESHOLD = 0.01

PAINT_COLOR = (255, 0, 0)
PAINT_SIZE = 1

TEXT_COLOR = (255, 0, 0)

# fontpath = "./assets/NanumGothic.ttf"
# TEXT_FONT = ImageFont.truetype(fontpath, 20)
TEXT_FONT = cv2.FONT_HERSHEY_PLAIN

FONT_SIZE = 1
FONT_WEIGHT = 1


def draw_boxes(image, boxes):
    k = 0
    # for box in boxes:
    for box in boxes:

        label = box.label
        conf = np.around(box.conf, 2)

        ymin = box.rect.yline.low
        xmin = box.rect.xline.low
        ymax = box.rect.yline.high
        xmax = box.rect.xline.high

        cv2.rectangle(
            image,
            (xmin, ymin),
            (xmax, ymax),
            PAINT_COLOR,
            PAINT_SIZE
        )
        cv2.putText(
            image,
            box.name + ' ' + str(conf),
            (xmin, ymin - 1),
            TEXT_FONT,
            FONT_SIZE,
            TEXT_COLOR,
            FONT_WEIGHT,
            cv2.LINE_AA
        )
        k += 1

    return image


def create_detector(model_path, label_path, model_confidence, num_classes):

    label_map = label_map_util.load_labelmap(label_path)
    categories = label_map_util.convert_label_map_to_categories(
        label_map,
        max_num_classes=num_classes,
        use_display_name=True
    )
    category_index = label_map_util.create_category_index(categories)
    detection_graph = tf.Graph()
    with detection_graph.as_default():
        od_graph_def = tf.GraphDef()
        with tf.gfile.GFile(model_path, 'rb') as fid:
            serialized_graph = fid.read()
            od_graph_def.ParseFromString(serialized_graph)
            tf.import_graph_def(od_graph_def, name='')
        image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')
        d_boxes = detection_graph.get_tensor_by_name('detection_boxes:0')
        d_scores = detection_graph.get_tensor_by_name('detection_scores:0')
        d_classes = detection_graph.get_tensor_by_name('detection_classes:0')
        num_d = detection_graph.get_tensor_by_name('num_detections:0')

    sess = tf.Session(graph=detection_graph)

    def detect(img):
        with detection_graph.as_default():
            img_expanded = np.expand_dims(img, axis=0)
            (boxes, scores, classes, num) = sess.run(
                [
                    d_boxes,
                    d_scores,
                    d_classes,
                    num_d
                ],
                feed_dict={image_tensor: img_expanded}
            )

            picked = list(filter(lambda x: x > model_confidence, scores[0]))
            picked_size = len(picked)
            _boxes = boxes[0][:picked_size]
            _classes = []
            _labels=[]
            clsids = classes[0][:picked_size]
            for k in clsids:
                if k in category_index.keys():
                    class_name = category_index[k]['name']
                    class_id = category_index[k]['id']
                    _classes.append(class_name)
                    _labels.append(class_id)
                else:
                    _classes.append('0') #Sean debug : add nul class name to sync array size
                    _labels.append(0)
            _scores = scores[0][:picked_size]
            return _boxes, _classes,_labels, _scores

    return detect
