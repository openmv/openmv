import mtx, vec

class Planar(object):
    def __init__(self, H, offset=[0,0]):
        self.H = H
        self.H_ = None  # LU factorization of H
        self.offset = offset

    def project(self, points, reverse=False):
        if reverse:
            if self.H_ is None:
                self.H_ = mtx.lu([r[:] for r in self.H])

            qs = [mtx.solve(self.H_,
                        vec.sub(p, self.offset)+[1]) for p in points]
            return [vec.div(q[:-1], q[-1]) for q in qs]
        else:
            qs = [mtx.mul(self.H, p+[1]) for p in points]
            return [vec.add(self.offset,
                        vec.div(q[:-1], q[-1])) for q in qs]
