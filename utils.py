# -*- coding: utf-8 -*-

#from matplotlib import rcParams
import matplotlib.pyplot as plt
import matplotlib.font_manager as fm
import numpy as np

import geo
import ssd
import os

import functools

# font_list = fm.findSystemFonts(fontpaths=None, fontext='ttf')
# font_list.sort()
# for font in font_list:
#     if 'Nanum' in font:
#         print (font)

# Say, "the default sans-serif font is COMIC SANS"
# rcParams['font.sans-serif'] = "Comic Sans MS"
# # Then, "ALWAYS use sans-serif fonts"
# rcParams['font.family'] = "sans-serif"

# Set Korean font

font_name = fm.FontProperties(fname='C:\\WINDOWS\\Fonts\\NGULIM.TTF',size=100).get_name()

plt.rc('font', family=font_name)

# print('# 설정 되어있는 폰트 사이즈')
# print (plt.rcParams['font.size'] )
# print('# 설정 되어있는 폰트 글꼴')
# print (plt.rcParams['font.family'] )

PLATE_DIGITS = {'0','1','2','3','4','5','6','7','8','9'}


PLATE_REGIONS = set([x for x in (
    '서울', '대구', '광주', '울산', '경기',
    '충북', '전북', '경북', '제주', '부산',
    '인천', '대전', '세종', '강원', '충남',
    '전남', '경남'
)])

PLATE_REGION_HANGULS = set((c for x in PLATE_REGIONS for c in x))

PLATE_MID_HANGULS = set([x for x in (
    '가', '나', '다', '라', '마', '바', '사', '아', '자', '차', '카', '타', '파', '하',
    '거', '너', '더', '러', '머', '버', '서', '어', '저', '처', '커', '터', '퍼', '허',
    '고', '노', '도', '로', '모', '보', '소', '오', '조', '초', '코', '토', '포', '호',
    '구', '누', '두', '루', '무', '부', '수', '우', '주', '추', '쿠', '투', '푸', '후',
    '배', '-', '크'
)])

#--SEAN

PLATE_HANGULS = PLATE_REGION_HANGULS.union(PLATE_MID_HANGULS)

PLATE_YOUNG = '영'

#++SEAN : region name 재조정

def cmp(a, b):
    return (a > b) - (a < b)


def remove_extension(filename):
    return filename.rsplit('.', 1)[0]

def is_plate_region(region):
    return region in PLATE_REGIONS

def is_plate_digits(digits):
    for c in digits:
        if c not in PLATE_DIGITS:
            return False
    return True

def is_plate_mid_hangul(hangul):
    return hangul in PLATE_MID_HANGULS

def is_plate_hangul(hangul):
    return hangul in PLATE_HANGULS

def is_plate_young(hangul):
    return hangul == PLATE_YOUNG

def is_valid_plate(plate):
    # The plate starts with digits or region
    if not is_plate_digits(plate[0]):
        region = plate[:2]
        if not is_plate_region(region):
            return False
        plate = plate[:2]

    # 2 digits
    if not is_plate_digits(plate[:2]):
        return False
    plate = plate[2:]

    # 1 hangul (including dash)
    if not is_plate_hangul(plate[0]):
        return False
    plate = plate[1:]

    # 4 digits
    if not is_plate_digits(plate[:4]):
        return False
    plate = plate[4:]

    if len(plate) == 0:
        return True

    if len(plate) == 1:
        return plate[0] == PLATE_YOUNG

    return False

def filter_bbox_ios(bboxes, ios_threshold):
    bboxes = bboxes[:]

    bboxes.sort(key=functools.cmp_to_key(ssd.BBox.xcmp))
    left_idx = 0
    while left_idx < len(bboxes)-1:
        right_idx = left_idx + 1
        while right_idx < len(bboxes):
            left = bboxes[left_idx]
            right = bboxes[right_idx]

            ios = geo.rect_ios(left.rect, right.rect)
            if ios < ios_threshold:
                right_idx += 1
                continue

            if left.conf == right.conf:
                victim = min([left, right], key=lambda x: x.area)
            else:
                victim = min([left, right], key=lambda x: x.conf)

            #victim = min([left, right], key=lambda x: x.area)
            if victim is left:
                bboxes.pop(left_idx)
                break
            bboxes.pop(right_idx)

        left_idx += 1
    return bboxes

def filter_bbox_iou(bboxes, iou_threshold):
    bboxes = bboxes[:]

    bboxes.sort(key=functools.cmp_to_key(ssd.BBox.xcmp))
    left_idx = 0
    while left_idx < len(bboxes)-1:
        right_idx = left_idx + 1
        while right_idx < len(bboxes):
            left = bboxes[left_idx]
            right = bboxes[right_idx]

            iou = geo.rect_iou(left.rect, right.rect)
            if iou < iou_threshold:
                right_idx += 1
                continue

            if left.conf == right.conf:
                victim = min([left, right], key=lambda x: x.area)
            else:
                victim = min([left, right], key=lambda x: x.conf)

            if victim is left:
                bboxes.pop(left_idx)
                break
            bboxes.pop(right_idx)

        left_idx += 1
    return bboxes
#Sean block it to avoidd py2exe error

def draw_bboxes(image, bboxes):
    plt.imshow(image)
    current_axis = plt.gca()

    colors = plt.cm.hsv(np.linspace(0, 1, len(bboxes)))
    for i, bbox in enumerate(bboxes):
        xmin = bbox.rect.xline.low
        xmax = bbox.rect.xline.high
        ymin = bbox.rect.yline.low
        ymax = bbox.rect.yline.high
        coords = (xmin, ymin), xmax-xmin+1, ymax-ymin+1

        rect_box = plt.Rectangle(*coords, fill=False, edgecolor=colors[i], linewidth=2)
        current_axis.add_patch(rect_box)
        if 0:
            if bbox.conf < 0.4 :
                current_axis.text(xmin, ymin+ymax+(bbox.conf)*20, '({}){}({:.2f})'.format(i, bbox.name.encode('utf-8'), bbox.conf).decode('utf-8'), bbox={'facecolor': colors[i], 'alpha': 0.5})
            #elif bbox.conf > 0.3 :
            elif bbox.conf >= 0.4 and bbox.conf < 0.7 :
                current_axis.text(xmin, ymin-(bbox.conf)*20, '({}){}({:.2f})'.format(i, bbox.name.encode('utf-8'), bbox.conf).decode('utf-8'), bbox={'facecolor': colors[i], 'alpha': 0.5})
            else :
                current_axis.text(xmin, ymin, '({}){}({:.2f})'.format(i, bbox.name.encode('utf-8'), bbox.conf).decode('utf-8'), bbox={'facecolor': colors[i], 'alpha': 0.5})
        else:
            if bbox.conf < 0.4:
                current_axis.text(xmin, ymin + ymax + (bbox.conf) * 20,
                                  '(%s)%s(%.2f)'%(i, bbox.name, bbox.conf),
                                  bbox={'facecolor': colors[i], 'alpha': 0.5})
                # elif bbox.conf > 0.3 :
            elif bbox.conf >= 0.4 and bbox.conf < 0.7:
                current_axis.text(xmin, ymin - (bbox.conf) * 20,
                                  '(%s)%s(%.2f)'%(i, bbox.name, bbox.conf),
                                  bbox={'facecolor': colors[i], 'alpha': 0.5})
            else:
                current_axis.text(xmin, ymin,
                                  '(%s)%s(%.2f)'%(i, bbox.name, bbox.conf),
                                  bbox={'facecolor': colors[i], 'alpha': 0.5})


