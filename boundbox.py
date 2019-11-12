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


def cmp(a, b):
    if a > b:
        return 1
    elif a < b:
        return -1
    else:
        return 0