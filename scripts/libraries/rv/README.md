# OpenRV - Robot Vision routines for OpenMV

[OpenMV](https://openmv.io) is a small camera board for machine vision. I use it
to direct my robot. [The software
library](http://docs.openmv.io/library/index.html) is adequate, but lacks some
algorithms I need. Out comes this project, in which I implement a few missing
pieces myself. I hope it could save time for someone who tries to do the same.

**Everything is tested on OpenMV Cam H7**

## Prerequisites

Vector and matrix are mainstay in machine vision. Finding no satisfactory
libraries, I have again implemented my own.

#### Vector Operations on MicroPython

- [Project](https://gitlab.com/nickoala/micropython-vec)
- [Package](https://pypi.org/project/micropython-vec/)

#### Fast Matrix Multiplication and Linear Solver on MicroPython

- [Project](https://gitlab.com/nickoala/micropython-mtx)
- [Package](https://pypi.org/project/micropython-mtx/)

Along with this project's `rv` package, the OpenMV camera's SD card should
contain:

```
/
├── mtx.py
├── rv
│   ├── __init__.py
│   ├── moments.py
│   ├── planar.py
│   └── quickshiftpp.py
└── vec
    ├── distance.py
    └── __init__.py
```

**In addition, you need some theoretical backgrounds to use vision algorithms
effectively. This page does not give you those backgrounds. Study them
yourself.**

## Hu moments

[Hu moments](https://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html#humoments)
[is a shape descriptor](https://www.pyimagesearch.com/2014/10/27/opencv-shape-descriptor-hu-moments-example/)
[invariant to translation, scale, and rotation](https://www.learnopencv.com/shape-matching-using-hu-moments-c-python/).
That means it can recognize the same shape no matter its location, size, and orientation in the picture.

#### Usage

The last element of Hu moments is somewhat of an oddball. It indicates
reflection rather than the general shape. For matching, the last element should
be dropped.

Use the function `vec.distance.euclidean()` to see how close two vectors are.

```python
import rv.moments
import vec.distance
import image

a = image.Image('/images/a.pgm')
b = image.Image('/images/b.pgm')

ha = rv.moments.hu(a)
hb = rv.moments.hu(b)

print(vec.distance.euclidean(ha[:-1], hb[:-1]))
```

**Remark:** Although accepting gray-level images, this implementation treats
pixels as either 0 or 1. Pixels having a non-zero brightness are treated as 1.
This speeds up calculation.

**More:** [test_moments.py](test/test_moments.py)

## Planar homography

Map points from one coordinate system to another. For example, a red ball sits
at (90, 50) on the image and you know it is on the floor (not floating in air),
planar homography can map the image point (90, 50) to a position on the floor,
telling you how far the red ball is in front of the robot and how much left or
right. In this case, points are essentially mapped from the *image coordinate
system* to the *floor coordinate system*.

Once you can map points, finding out the size of objects is straight-forward.

#### Usage

It works only when two coordinate systems (i.e. the two planes) are fixed
relative to each other. In other words, the camera's height and orientation
relative to the floor cannot change.

**[First, you have to calibrate for a homography matrix, which is a very long
story. I have devoted an entire directory to discuss the process. Take a look
there.](homography)**

Once you have the matrix, the rest is easy.

```python
import rv.planar

H = [[ 3.14916496e+01, -9.79038178e+02,  1.03951636e+05],
     [ 7.57939015e+02, -3.31912533e+01, -5.86807545e+04],
     [ 2.06572544e-01,  2.03579263e+00,  1.00000000e+00]]

p = rv.planar.Planar(H)

image_points = [[83, 109],
                [70, 100],
                [51,  92]]

print(p.project(image_points))
```

**More:** [test_planar.py](test/test_planar.py)

## Quickshift++

[The latest member](https://github.com/google/quickshift) [of the
Meanshift](http://www.chioka.in/meanshift-algorithm-for-the-rest-of-us-python/)
[family of clustering
algorithms](https://github.com/Nick-Ol/MedoidShift-and-QuickShift), Quickshift++
accepts a bunch of points and group them. I use it to "discover" the colors of
disks on the floor, before using colors to pick out the disks. This saves me
from hard-coding the colors beforehand, and makes the robot adaptive.

It is not optimized to handle a large number of points. OpenMV's limited memory
precludes handling a lot of points anyway. Don't expect to use it to segment an
entire image.

#### Usage

```python
import rv.quickshiftpp

points = [
    # cluster 1
    [1, 1],
    [1.0, 1.2],
    [0.9, 1.1],
    [0.95, 0.99],

    # cluster 2
    [3.3, 3.0],

    # cluster 3
    [5.0, 8.2],
    [5.5, 7.9],
    [4.8, 8.1],
    [5.1, 7.7],
]

print(rv.quickshiftpp.cluster(points,
                              k=2,
                              beta=0.2))
```

The parameter `k` determines how density is estimated. It uses *distance to the
k-th nearest neighbor* to estimate density around each point.

The parameter `beta` determines how much density is allowed to vary within
cluster cores. Here is not the place to explain what "cluster core" means. Some
theoretical understanding cannot be avoided.

In short, use `k` and `beta` to tune the clustering.

**More:** [test_quickshiftpp.py](test/test_quickshiftpp.py) and
          [test_quickshiftpp_colors.py](test/test_quickshiftpp_colors.py)
