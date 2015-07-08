#!/usr/bin/env python2.7
import sys,os
import struct
import argparse
from xml.dom import minidom


def cascade_info(path):
    #parse xml file
    xmldoc = minidom.parse(path)

    trees = xmldoc.getElementsByTagName('trees')
    n_stages = len(trees)

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
    fout = open(os.path.basename(path).split('.')[0]+".cascade", "w")

    n_rectangles = 0
    for f in feature:
        rects = f.getElementsByTagName('_')
        n_rectangles = n_rectangles + len(rects)

    #print some cascade info
    print("size:%dx%d"%(size[0], size[1]))
    print("stages:%d"%len(stages))
    print("features:%d"%n_features)
    print("rectangles:%d"%n_rectangles)

def cascade_binary(path, n_stages, name):
    #parse xml file
    xmldoc = minidom.parse(path)

    trees = xmldoc.getElementsByTagName('trees')
    max_stages = len(trees)
    if n_stages > max_stages:
        raise Exception("The max number of stages is: %d"%(max_stages))

    if n_stages == 0:
        n_stages = max_stages

    # read stages
    stages = [len(t.childNodes)/2 for t in trees][0:n_stages]
    stage_threshold = xmldoc.getElementsByTagName('stage_threshold')[0:n_stages]

    # total number of features
    n_features = sum(stages)

    # read features threshold
    threshold = xmldoc.getElementsByTagName('threshold')[0:n_features]

    # theres one of each per feature
    alpha1 = xmldoc.getElementsByTagName('left_val')[0:n_features]
    alpha2 = xmldoc.getElementsByTagName('right_val')[0:n_features]

    # read rectangles
    feature = xmldoc.getElementsByTagName('rects')[0:n_features]

    # read cascade size
    size = (map(int, xmldoc.getElementsByTagName('size')[0].childNodes[0].nodeValue.split()))

    # open output file with the specified name or xml file name
    if not name:
        name = os.path.basename(path).split('.')[0]
    fout = open(name+".cascade", "w")

    n_rectangles = 0
    for f in feature:
        rects = f.getElementsByTagName('_')
        n_rectangles = n_rectangles + len(rects)

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

    # print cascade info
    print("size:%dx%d"%(size[0], size[1]))
    print("stages:%d"%len(stages))
    print("features:%d"%n_features)
    print("rectangles:%d"%n_rectangles)
    print("binary cascade generated")

def main():
    # CMD args parser
    parser = argparse.ArgumentParser(description='haar cascade generator')
    parser.add_argument("-i", "--info",     action = "store_true",  help = "print cascade info and exit")
    parser.add_argument("-n", "--name",     action = "store",       help = "set cascade name", default = "")
    parser.add_argument("-s", "--stages",   action = "store",       help = "set the maximum number of stages", type = int, default=0)
    parser.add_argument("file", action = "store", help = "OpenCV xml cascade file path")

    # Parse CMD args
    args = parser.parse_args()

    if args.info:
        # print cascade info and exit
        cascade_info(args.file)
        return

    # generate a binary cascade from the xml cascade
    cascade_binary(args.file, args.stages, args.name)

if __name__ == '__main__':
    main()
