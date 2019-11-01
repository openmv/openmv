import math

def _sum(img, roi, f):
    x, y, w, h = roi or (0, 0, img.width(), img.height())
    s = 0
    for i in range(x, x + w):
        for j in range(y, y + h):
            if img.get_pixel(i,j) != 0:
                s += f(i, j)
    return s

def centroid(img, roi=None, return_intermediate=False):
    m10 = _sum(img, roi, lambda x,y: x)
    m01 = _sum(img, roi, lambda x,y: y)
    m00 = _sum(img, roi, lambda x,y: 1)

    c = [0, 0] if m00 == 0 else [float(m10 / m00), float(m01 / m00)]

    return [m00, c] if return_intermediate else c

def central(img, roi=None, return_intermediate=False):
    results = centroid(img, roi, return_intermediate=True)
    cx, cy = results[-1]

    u20 = _sum(img, roi, lambda x,y: (x-cx)**2)
    u11 = _sum(img, roi, lambda x,y: (x-cx) * (y-cy))
    u02 = _sum(img, roi, lambda x,y: (y-cy)**2)

    u30 = _sum(img, roi, lambda x,y: (x-cx)**3)
    u21 = _sum(img, roi, lambda x,y: (x-cx)**2 * (y-cy))
    u12 = _sum(img, roi, lambda x,y: (x-cx) * (y-cy)**2)
    u03 = _sum(img, roi, lambda x,y: (y-cy)**3)

    us = [u20, u11, u02, u30, u21, u12, u03]

    return (results + [us]) if return_intermediate else us

def normalized_central(img, roi=None, return_intermediate=False):
    results = central(img, roi, return_intermediate=True)
    m00, _, [u20, u11, u02, u30, u21, u12, u03] = results

    if m00 == 0:
        ns = [0, 0, 0, 0, 0, 0, 0]
    else:
        m2 = m00**2
        m3 = m00**2.5
        n20, n11, n02      = u20/m2, u11/m2, u02/m2
        n30, n21, n12, n03 = u30/m3, u21/m3, u12/m3, u03/m3

        ns = [n20, n11, n02, n30, n21, n12, n03]

    return (results + [ns]) if return_intermediate else ns

def hu(img, roi=None, return_intermediate=False):
    results = normalized_central(img, roi, return_intermediate=True)
    n20, n11, n02, n30, n21, n12, n03 = results[-1]

    h0 = n20 + n02

    h1 = (n20 - n02)**2 + 4 * n11 * n11

    _n30_3n12 = n30 - 3 * n12
    _3n21_n03 = 3 * n21 - n03

    h2 = _n30_3n12**2 + _3n21_n03**2

    n3012 = n30 + n12
    n3012_2 = n3012 * n3012

    n2103 = n21 + n03
    n2103_2 = n2103 * n2103

    h3 = n3012_2 + n2103_2

    h4 = (_n30_3n12 * n3012 * ( n3012_2 - 3 * n2103_2 )
        + _3n21_n03 * n2103 * ( 3 * n3012_2 - n2103_2 ))

    h5 = ( n20 - n02 ) * ( n3012_2 - n2103_2 ) + 4 * n11 * n3012 * n2103

    h6 = (_3n21_n03 * n2103 * ( 3 * n3012_2 - n2103_2 )
        - _n30_3n12 * n2103 * ( 3 * n3012_2 - n2103_2 ))

    hs = [0 if h == 0 else math.copysign(math.log10(math.fabs(h)), h)
                        for h in [h0, h1, h2, h3, h4, h5, h6]]

    return (results + [hs]) if return_intermediate else hs
