# -*- coding: utf-8 -*-

from __future__ import division

class Line(object):
    def __init__(self, low, high):
        self.low = low
        self.high = high

        assert self.low <= self.high, \
               'Invalid line argument: {} > {}'.format(self.low, self.high)

    @property
    def length(self):
        return (self.high - self.low + 1)

    @property
    def center(self):
        return (self.high + self.low) / 2

class Rect(object):
    def __init__(self, xline, yline):
        self.xline = xline
        self.yline = yline

    @property
    def center(self):
        return (self.xline.center, self.yline.center)

    @property
    def area(self):
        return self.xline.length * self.yline.length

    @property
    def roi(self):
        return (self.xline.low, self.yline.low, self.xline.length, self.yline.length)

def line_intersection(line1, line2):
    if line1.low > line2.low:
        line1, line2 = line2, line1

    if line1.high < line2.low:
        return None

    low = line2.low
    high = line1.high if line1.high < line2.high else line2.high

    return Line(low, high)

def rect_intersection(rect1, rect2):
    xline = line_intersection(rect1.xline, rect2.xline)
    yline = line_intersection(rect1.yline, rect2.yline)

    if xline is None or yline is None:
        return None

    return Rect(xline, yline)

def line_iou(line1, line2):
    intersection = line_intersection(line1, line2)
    if intersection is None:
        return 0

    return intersection.length / (line1.length + line2.length - intersection.length)

def line_ios(line1, line2):
    intersection = line_intersection(line1, line2)
    if intersection is None:
        return 0

    small = line1 if line1.length < line2.length else line2
    return intersection.length / small.length

def rect_iou(rect1, rect2):
    intersection = rect_intersection(rect1, rect2)
    if intersection is None:
        return 0

    #return intersection.area / (rect1.area + rect1.area - intersection.area)
    return intersection.area / (rect1.area + rect2.area - intersection.area)# Sean change the bug typo

def rect_ios(rect1, rect2):
    intersection = rect_intersection(rect1, rect2)
    if intersection is None:
        return 0

    small = rect1 if rect1.area < rect2.area else rect2
    return intersection.area / small.area
