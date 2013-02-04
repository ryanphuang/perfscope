#!/usr/bin/python
import glob
import os

def process(fname):
    fp = open(fname,'r')
    funcs = []
    called = []
    loops = False
    for line in fp:
        line = line.strip()
        if line.find("$$") >= 0:
            grps = line.split("$$")
            left = grps[0]
            right = grps[1]
            idx = left.find(':')
            if idx >= 0:
                funcs.append(left[:idx])
                if left[idx + 1:].find(",") >= 0:
                    ffs = left[idx + 1:].split(",")
                    for ff in ffs:
                        ff = ff.strip()
                        if len(ff) > 0:
                            called.append(ff)
            if right.find("[#") >= 0:
                loops = True
    print fname + "\t" + str(loops) + "\t" + ",".join(funcs) + "\t" + ",".join(called)

for f in glob.glob("*.result"):
    if f.endswith(".result"):
        process(f)
