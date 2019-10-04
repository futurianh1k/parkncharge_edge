#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import division
from __future__ import print_function

import inspect
import os
import sys

import cv2
import datetime
import shutil
import traceback

# from scipy.misc import imsave
from matplotlib import font_manager as fm
from matplotlib import rc

import geo
import boundbox as bb
import tfutils
# from errors import LPRError, LPRNoPlateError, LPRParseError
# Need to import necerssary modules and functions only
# import caffe
# from caffe import set_mode_cpu, set_mode_gpu, set_device
# import skimage.util

import skimage.io as io
from skimage.io import imread, imsave, use_plugin
from skimage import img_as_float, img_as_ubyte

# Error correction for
# You may load I/O plugins with the `skimage.io.use_plugin` command. A list of all available plugins are shown in the `skimage.io` docstring.

#io.use_plugin('pil')

import numpy as np

import skimage.exposure
import skimage.color
from skimage.color import rgb2gray
from skimage.color import gray2rgb

from PIL import ImageFile
import platform
import functools

# 2019.03.29 added
import time
#import shutil
os.environ["CUDA_VISIBLE_DEVICES"] = '0'
#
ImageFile.LOAD_TRUNCATED_IMAGES = True  ### SEAN : to avoid loading failure of truncated images
#


# 2019.03.20 added########################################
# total_process_time = 0
# total_image_load_time = 0
# total_image_save_time = 0
# total_detection_time = 0

# Error : 001~099 : operation errors
#       : 100~199 : Localizer errors
#       : 200~299 : Recognizer error
Error_Table = {'000': 'No error',
               '001': 'Cannot load image',
               '002': 'No GT file',
               '003': 'Miss-matched',
               '100': 'Localizer Error',
               '101': 'LOC : Cannot fine any plates in a full image',
               '102': 'LOC : Cannot choose a major plate in candidate',
               '200': 'RECOG : Recognizer Error',
               '201': 'RECOG : No Hangul',
               '202': 'RECOG : No digits',
               '203': 'RECOG : Missing some digits',
               '204': 'RECOG : No plate string',
               '205': 'RECOG : Missing some digits, Need to extend left and right',
               '206': 'RECOG : Missing some digits need to extend right margin',
               '207': 'RECOG : Missing hangul need to extend left margin',
               '208': 'RECOG : No Region'
               }


def load_image(filename, color=True):
    """
    Load an image converting from grayscale or alpha as needed.

    Parameters
    ----------
    filename : string
    color : boolean
        flag for color format. True (default) loads as RGB while False
        loads as intensity (if image is already grayscale).

    Returns
    -------
    image : an image with type np.float32 in range [0, 1]
        of size (H x W x 3) in RGB or
        of size (H x W x 1) in grayscale.
    """
    img = img_as_float(imread(filename, as_grey=not color)).astype(np.float32)
    if img.ndim == 2:
        img = img[:, :, np.newaxis]
        if color:
            img = np.tile(img, (1, 1, 3))
    elif img.shape[2] == 4:
        img = img[:, :, :3]
    return img_as_ubyte(img)  # to convert CV image


### Localization input a full_image return possible plate images array and plate type
def localization(localizer, fullImage, l_ios, l_iou, log):
    plateImage = []
    bbConf = 0.0
    plateType = ''
    lError = ''
    lBbox = None

    ### Localizer Detection ###
    lBboxes = _predict(fullImage, localizer, True, log)
    if len(lBboxes) > 0:
        plateImages, bbConfs, plateTypes, lBboxes = get_plates(fullImage, lBboxes, l_ios, l_iou)
        plateImage, bbConf, plateType, lBbox = get_major_plate(plateImages, bbConfs, plateTypes, lBboxes, log)

    if lBbox:
        lError = ''

    else:
        lError = Error_Table['101']  # '101':'LOC : Cannot fine any plates in a full image',


    #cv2.imwrite("py_result" + str(time.time())+".jpg", plateImage)
    return plateImage, bbConf, plateType, lError, lBbox


def recognition(recognizer, plateImage, rIos, rIou, plateType, log):
    rBboxes = _predict(plateImage, recognizer, False, log)

    #plateString, rError, rBboxes, rValidBboxes = get_plate_string(rBboxes, rIos, rIou, plateType, log)
    plateString, rError, rBboxes, rValidBboxes = get_plate_string(rBboxes, rIos, rIou, plateType, log)
    return plateString, rError, rBboxes, rValidBboxes


# ### Get multiple possible plates, return plats array and plate type
def get_plates(fullImage, bboxes, ios_threshold, iou_threshold):
    plateImages = []
    bbConfs = []
    plateTypes = []

    bboxes = filter_bbox(bboxes, ios_threshold, iou_threshold)

    for i in range(len(bboxes)):
        rect = bboxes[i].rect
        xmin, xmax = rect.xline.low, rect.xline.high
        ymin, ymax = rect.yline.low, rect.yline.high

        plateImages.append(fullImage[ymin:ymax + 1, xmin:xmax + 1])
        bbConfs.append(bboxes[i].conf)
        plateTypes.append(bboxes[i].name)

    return plateImages, bbConfs, plateTypes, bboxes

#
def filter_bbox(bboxes, ios_threshold, iou_threshold):
    bboxes = tfutils.filter_bbox_iou(bboxes, iou_threshold)
    bboxes = tfutils.filter_bbox_ios(bboxes, ios_threshold)
    return bboxes

#
# ### Choose a plate from detected plates array
def get_major_plate(plateImages, bbConfs, plateTypes, l_bboxes, log):
    # find max confidence index
    max_conf = bbConfs[0]  # init
    max_conf_idx = 0

    # tee(log, "Plate bbConf [{}:{}]".format(0, bbConfs[0]))
    if len(bbConfs) > 1:
        for i in range(len(bbConfs) - 1):
            # tee(log, "Plate bbConf [{}:{}]".format(i + 1, bbConfs[i + 1]))
            if max_conf <= bbConfs[i + 1]:
                max_conf = bbConfs[i + 1]
                max_conf_idx = i + 1
        # tee(log, "Plate choose[{}:{}]".format(max_conf_idx, bbConfs[max_conf_idx]))

    return plateImages[max_conf_idx], bbConfs[max_conf_idx], plateTypes[max_conf_idx], l_bboxes[max_conf_idx]

#
def get_plate_string(rBboxes, rIos, rIou, plateType, log):
    bboxes = filter_bbox(rBboxes, rIos, rIou)

    # filename=str(datetime.datetime.now().microsecond)+".jpg"

    hangul_bboxes = [bbox for bbox in bboxes if not tfutils.is_plate_digits(bbox.name)]  # for Korean chars
    digit_bboxes = [bbox for bbox in bboxes if tfutils.is_plate_digits(bbox.name)]  # for numbers

    region = ''
    hangul = ''
    young = ''

    digit_string, valid_digit_bboxes, is_old_plate = parse_digits(digit_bboxes, log)
    region, hangul, young, valid_kor_bboxes = parse_hanguls(hangul_bboxes, digit_string, log)

    valid_bboxes = valid_digit_bboxes
    for kor_bbox in valid_kor_bboxes:
        valid_bboxes.append(kor_bbox)

    parse_error = ''
    # print (plateType)

    if region == '' and hangul == '' and digit_string == '':
        parse_error = Error_Table['204']  # '204':'RECOG : No plate string'
        return region + digit_string[:2] + hangul + digit_string[2:] + young, parse_error, bboxes, valid_bboxes

    if len(bboxes) <= 4:
        parse_error = Error_Table['202']  # '202':'RECOG : No digits'
        return region + digit_string[:2] + hangul + digit_string[2:] + young, parse_error, bboxes, valid_bboxes

    if hangul == '':
        parse_error = Error_Table['201']  # '201':'RECOG : No Hangul',
        return region + digit_string[:2] + hangul + digit_string[2:] + young, parse_error, bboxes, valid_bboxes

    if plateType == 'class0' or plateType == 'class1':  # type T01,T02,T04
        if len(digit_string) < 6:
            if digit_string == '':
                parse_error = Error_Table['202']  # No digits
            else:
                parse_error = Error_Table['205']  # Missing some digits need to extend left and right


    elif plateType == 'class2':  # type T03,T06
        if len(digit_string) < 6:
            if digit_string == '':
                parse_error = Error_Table['202']  # No digits
            else:
                parse_error = Error_Table['206']  # Missing some digits need to extend right margin
        elif hangul == '':
            parse_error = Error_Table['207']  # Missing hangul need to extend left margin

    elif plateType == 'class5':  # type T17, T18
        if len(digit_string) < 6:
            if digit_string == '':
                parse_error = Error_Table['202']  # No digits
            else:
                parse_error = Error_Table['206']  # Missing some digits need to extend right margin
        elif hangul == '':
            parse_error = Error_Table['207']  # Missing hangul need to extend left margin


    elif plateType == 'class3':
        if len(digit_string) < 6:
            if digit_string == '':
                parse_error = Error_Table['202']  # No digits
            else:
                parse_error = Error_Table['205']  # Missing some digits need to extend left and right

    if is_old_plate:
        return (region + digit_string[0] + hangul + digit_string[-4:] + young), parse_error, bboxes, valid_bboxes
    else:
        if len(digit_string) == 6:
            return (region + digit_string[:2] + hangul + digit_string[2:] + young), parse_error, bboxes, valid_bboxes
        elif len(digit_string) == 5:
            return (region + digit_string[0] + hangul + digit_string[1:] + young), parse_error, bboxes, valid_bboxes
        else:  # less than 5
            return (region + hangul + digit_string[0:] + young), parse_error, bboxes, valid_bboxes


def inference_an_image_simple(imgpath, localizer, recognizer, lIos=0.8, lIou=0.3, rIos=0.8, rIou=0.3):
    log = 1;
    result = ''
    print("enter inference")
    fullImage = load_image(imgpath)
    print("python image path : ", imgpath)
    ### Localizer Detection ###
    plateImage, bbConf, plateType, lError, lBbox = localization(localizer, fullImage, lIos, lIou, log)

    if lError:
        result= 'No plate'

    else:
        plateString, rError, rBboxes, rValidBboxes = recognition(recognizer, plateImage, rIos, rIou, plateType, log)
        result = plateString
    return result

def null_inference(imgPath, localizer, recognizer, lIos=0.8, lIou=0.3, rIos=0.8, rIou=0.3):
    log = 0
    basename = os.path.basename(imgPath)

    fullImage = load_image(imgPath)

    ### Localizer Detection ###
    plateImage, bbConf, plateType, lError, lBbox = localization(localizer, fullImage, lIos, lIou, log)

    plateString, rError, rBboxes, rValidBboxes = recognition(recognizer, plateImage, rIos, rIou,
                                                             plateType, log)

    return 0

def _predict(image, detect, isLoc, log):
    bboxes = []
    tfbboxes, names, labels, bbconfs = detect(image)

    h, w, d = image.shape

    # convert Bbox from tf to ssd type
    for id, tfbbox in enumerate(tfbboxes):
        xline = geo.Line(int(tfbbox[1] * w), int(tfbbox[3] * w))
        yline = geo.Line(int(tfbbox[0] * h), int(tfbbox[2] * h))
        rect = geo.Rect(xline, yline)
        bboxes.append(bb.BBox(rect, bbconfs[id], names[id], labels[id]))
    return bboxes


def parse_hanguls(hangul_bboxes, digit_string, log):
    region_bboxes = [bbox for bbox in hangul_bboxes if tfutils.is_plate_region(bbox.name)]
    young_bboxes = [bbox for bbox in hangul_bboxes if tfutils.is_plate_young(bbox.name)]
    # other_bboxes = [bbox for bbox in hangul_bboxes if utils.is_plate_hangul(bbox.name)]  # for utility code
    other_bboxes = [bbox for bbox in hangul_bboxes if tfutils.is_plate_mid_hangul(bbox.name)]  # for utility code

    valid_kor_bboxes = []

    region = ''
    hangul = ''
    young = ''

    if region_bboxes:
        max_region_conf = region_bboxes[0].conf  # init
        max_region_conf_idx = 0

        if len(region_bboxes) == 1:
            region = region_bboxes[0].name
            valid_kor_bboxes.append(region_bboxes[0])

        elif len(region_bboxes) > 1:
            for i in range(len(region_bboxes) - 1):
                if max_region_conf <= region_bboxes[i + 1].conf:
                    max_region_conf = region_bboxes[i + 1].conf
                    max_region_conf_idx = i + 1
            region = region_bboxes[max_region_conf_idx].name
            valid_kor_bboxes.append(region_bboxes[max_region_conf_idx])

    else:
        region = ''

    if young_bboxes:
        if region:  # SEAN : add young Heuristic :"영' should be with region
            if int(digit_string[-4:]) >= 5001 and int(digit_string[
                                                      -4:]) <= 8999:  # SEAN : add young Heuristic :'영' should be used between number 5001 to 899
                young = young_bboxes[0].name
                valid_kor_bboxes.append(young_bboxes[0])

        valid_kor_bboxes.append(young_bboxes[0])
    else:
        young = ''

    if other_bboxes:
        max_hangul_conf = other_bboxes[0].conf  # init
        max_hangul_conf_idx = 0

        if len(other_bboxes) == 1:
            hangul = other_bboxes[0].name
            valid_kor_bboxes.append(other_bboxes[0])
            # tee(log, ' Hangul conf[{}]: [{}] \'{}\''.format(0, other_bboxes[0].conf,other_bboxes[0].name))

        elif len(other_bboxes) > 1:
            for i in range(len(other_bboxes) - 1):
                # tee(log, ' Hangul conf[{}]: [{}] \'{}\''.format(i + 1, other_bboxes[i + 1].conf,other_bboxes[i + 1].name))

                if max_hangul_conf <= other_bboxes[i + 1].conf:
                    max_hangul_conf = other_bboxes[i + 1].conf
                    max_hangul_conf_idx = i + 1
            hangul = other_bboxes[max_hangul_conf_idx].name
            valid_kor_bboxes.append(other_bboxes[max_hangul_conf_idx])

            # tee(log, ' Hangul conf[{}]: [{}] \'{}\''.format(max_hangul_conf_idx, other_bboxes[max_hangul_conf_idx].conf,other_bboxes[max_hangul_conf_idx].name))

    else:  # No hangul bboxes
        hangul = ''

    return region, hangul, young, valid_kor_bboxes


def parse_digits(digit_bboxes, log):
    first_line_boxes = []
    second_line_boxes = []
    first_common_yline = None
    second_common_yline = None
    is_old_plate = 0  # to indicate old style 5 digits plate
    valid_digit_boxes = []

    if len(digit_bboxes) < 6:
        digit_bboxes = sorted(digit_bboxes, key=functools.cmp_to_key(bb.BBox.confcmp))[-len(digit_bboxes):]

    elif len(digit_bboxes) > 6:
        digit_bboxes = sorted(digit_bboxes, key=functools.cmp_to_key(bb.BBox.confcmp))[-6:]

    digit_bboxes.sort(key=functools.cmp_to_key(bb.BBox.ycmp))  # ymin sorted

    first_row_boxes = []
    second_row_boxes = []

    if 1:  # Sean new routin to find out if single or double lines
        xoverlapcnt = 0
        yoverlapcnt = 0
        top_yoverlapcnt = 0
        if len(digit_bboxes) <= 4:  # Too lack number of digits
            digit_bboxes.sort(key=functools.cmp_to_key(bb.BBox.xcmp))
            first_row_boxes = digit_bboxes
        else:

            # get the middle y
            top_Y = digit_bboxes[0].rect.yline.low  # highest y
            bottom_Y = digit_bboxes[-1].rect.yline.high  # lowest y
            mid_Y = int(round((top_Y + bottom_Y) / 2))
            lowerCnt = 0
            for box in digit_bboxes:
                if box.rect.yline.high > mid_Y:
                    lowerCnt += 1

            # check if upper digits located in lower digits' x range
            for box in digit_bboxes[:2]:
                for i in range(len(digit_bboxes)):
                    if int(round(box.center[0])) > digit_bboxes[i].rect.xline.low and int(round(box.center[0])) < \
                            digit_bboxes[i].rect.xline.high:
                        xoverlapcnt += 1

            # check if the number of upper digits are below 2
            # maximum 4 on  double lines
            for box in digit_bboxes[:2]:
                for i in range(len(digit_bboxes)):
                    if int(round(box.center[1])) > digit_bboxes[i].rect.yline.low and int(round(box.center[1])) < \
                            digit_bboxes[i].rect.yline.high:
                        yoverlapcnt += 1

            for i in range(len(digit_bboxes)):
                if int(round(digit_bboxes[0].center[1])) > digit_bboxes[i].rect.yline.low and int(
                        round(digit_bboxes[0].center[1])) < \
                        digit_bboxes[i].rect.yline.high:
                    top_yoverlapcnt += 1

            if lowerCnt >= 5 and yoverlapcnt >= 8:  # single line, means plate's slope is flat and 2 highest y bboxes are NOT existed
                first_row_boxes = digit_bboxes
            else:
                if top_yoverlapcnt == 1:  # xoverlapcnt <= 3 and len(digit_bboxes) <= 5:  # old plate
                    is_old_plate = 1
                    for i in range(1):
                        first_row_boxes.append(digit_bboxes[i])
                    for i in range(1, len(digit_bboxes)):
                        second_row_boxes.append(digit_bboxes[i])
                else:  # new double line
                    for i in range(2):
                        first_row_boxes.append(digit_bboxes[i])
                    for i in range(2, len(digit_bboxes)):
                        second_row_boxes.append(digit_bboxes[i])


    first_row_boxes.sort(key=functools.cmp_to_key(bb.BBox.xcmp))

    second_row_boxes.sort(key=functools.cmp_to_key(bb.BBox.xcmp))

    first_row_digits = ''.join([bbox.name for bbox in first_row_boxes])
    second_row_digits = ''.join([bbox.name for bbox in second_row_boxes])
    # print('First row = ', first_row_digits)
    # print('Second row = ', second_row_digits)
    valid_digit_boxes = first_row_boxes
    for second_box in second_row_boxes:
        valid_digit_boxes.append(second_box)
    return first_row_digits + second_row_digits, valid_digit_boxes, is_old_plate
