#!/usr/bin/env python
import sys,os
import struct
from xml.dom import minidom

def print_help_exit():
  print "usage: cascade.py <cv_haar_cascade.xml> <num stages>"
  sys.exit(1)

if len(sys.argv)!= 3:
    print_help_exit()

n_stages = int(sys.argv[2])
#parse xml file
xmldoc = minidom.parse(sys.argv[1])

trees = xmldoc.getElementsByTagName('trees')
max_stages = len(trees)
if n_stages > max_stages:
    raise Exception("The max number of stages is: %d"%(max_stages))

# read stages
stages = [len(t.childNodes)/2 for t in trees][0:n_stages]
stage_threshold = xmldoc.getElementsByTagName('stage_threshold')[0:n_stages]

# total number of features
n_features = sum(stages)
# read features threshold
threshold = xmldoc.getElementsByTagName('threshold')[0:n_features]
#theres one of each per feature
alpha1 = xmldoc.getElementsByTagName('left_val')[0:n_features]
alpha2 = xmldoc.getElementsByTagName('right_val')[0:n_features]

#read rectangles
feature = xmldoc.getElementsByTagName('rects')[0:n_features]

#read cascade size
size = (map(int, xmldoc.getElementsByTagName('size')[0].childNodes[0].nodeValue.split()))
fout = open(os.path.basename(sys.argv[1]).split('.')[0]+".cascade", "w")

n_rectangles = 0
for f in feature:
    rects = f.getElementsByTagName('_')
    n_rectangles = n_rectangles + len(rects)

#print some cascade info
print "size:%dx%d"%(size[0], size[1])
print "stages:%d"%len(stages)
print "features:%d"%n_features
print "rectangles:%d"%n_rectangles

# write detection window size
fout.write(struct.pack('i', size[0]))
fout.write(struct.pack('i', size[1]))

# write num stages
fout.write(struct.pack('i', len(stages)))

# write num feat in stages
for s in stages:
    fout.write(struct.pack('B', s)) # uint8_t

# write stages thresholds
for t in stage_threshold:
    fout.write(struct.pack('h', int(float(t.childNodes[0].nodeValue)*256))) #int16_t

# write features threshold 1 per feature
for t in threshold:
    fout.write(struct.pack('h', int(float(t.childNodes[0].nodeValue)*4096))) #int16_t

# write alpha1 1 per feature
for a in alpha1:
    fout.write(struct.pack('h', int(float(a.childNodes[0].nodeValue)*256))) #int16_t

# write alpha2 1 per feature
for a in alpha2:
    fout.write(struct.pack('h', int(float(a.childNodes[0].nodeValue)*256))) #int16_t

# write num_rects per feature
for f in feature:
    rects = f.getElementsByTagName('_')
    fout.write(struct.pack('B', len(rects))) # uint8_t

# write rects weights 1 per rectangle
for f in feature:
    rects = f.getElementsByTagName('_')
    for r in rects:
        l = map(int, r.childNodes[0].nodeValue[:-1].split())
        fout.write(struct.pack('b', l[4])) #int8_t NOTE: multiply by 4096

# write rects
for f in feature:
    rects = f.getElementsByTagName('_')
    for r in rects:
        l = map(int, r.childNodes[0].nodeValue[:-1].split())
        fout.write(struct.pack('BBBB',l[0], l[1], l[2], l[3])) #uint8_t
