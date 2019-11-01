# Vector Operations on MicroPython

Nothing fancy. Treating list as vector, this library performs common
operations. I personally use it on [OpenMV](https://openmv.io) for robot vision.

```python
import vec

a = [1, 2, 3]
b = [4, 5, 6]

vec.add(a, b)  # [5, 7, 9]
vec.sub(a, b)  # [-3, -3, -3]
vec.dot(a, b)  # 32

vec.mul(a, 2)  # [2, 4, 6]
vec.div(a, 2)  # [0.5, 1.0, 1.5]
```

```python
import vec.distance

a = [2, 3]
b = [5, 7]

vec.distance.manhattan(a, b)  # 7
vec.distance.euclidean(a, b)  # 5
```
