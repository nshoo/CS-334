from time import sleep
from turtle import clear
from PIL import Image
import math

def clamp(val, low, high):
    return max(min(val, high), low)

class Grid:
    def __init__(self, size):
        self.size = size
        self.list = [[0.0 for x in range(size)] for y in range(size)]
    
    def display(self):
        for y in range(self.size):
            print(' '.join([str("%.1f" % v) for v in self.list[y]]))

    def set(self, x, y, val):
        self.list[y][x] = clamp(val, 0.0, 1.0)
    
    def get(self, x, y):
        return self.list[y][x]

    def in_bounds(self, pt):
        return pt[0] >= 0 and pt[0] < self.size and pt[1] >= 0 and pt[1] < self.size

    def get_neighbors(self, x, y):
        offsets = [(-1, 0), (1, 0), (0, 1), (0, -1)]
        points = map(lambda off: (x + off[0], y + off[1]), offsets)
        return list(filter(lambda pt: self.in_bounds(pt), points))

    def get_neighbor_sum(self, x, y):
        points = self.get_neighbors(x, y)
        values = [self.get(pt[0], pt[1]) for pt in points]
        return sum(values)

    def display_image(self):
        img_size = self.size * 10
        img = Image.new("RGB", (img_size, img_size))
        pix = img.load()

        for y in range(img_size):
            for x in range(img_size):
                strength = self.get(math.floor(x / 10), math.floor(y / 10))
                pix[x, y] = (0, 0, int(255 * strength))
        img.show()

def propagate(left, right):
    size = left.size
    for y in range(size):
        for x in range(size):
            sum = left.get_neighbor_sum(x, y)
            right.set(x, y, (sum / (2 - right.get(x, y))) * 0.95)

one = Grid(10)
two = Grid(10)

one.set(5, 5, 1.0)

swap = False

for i in range(20):
    one.display()
    print()
    one.display_image()

    if swap:
        propagate(two, one)
    else:
        propagate(one, two)
    swap = not swap

    sleep(2.0)