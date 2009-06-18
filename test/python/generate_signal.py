#!/usr/bin/env python
import struct
import numpy
from optparse import OptionParser
import sys

op = OptionParser()
op.add_option("-l", dest="size", type="int", default=1024)
op.add_option("-s", dest="step", type="float", default=0.1)
op.add_option("-n", dest="noise_level", type="float", default=10.0)
op.add_option("-o", dest="filename", default="signal.dat")
opts, args = op.parse_args(sys.argv[1:])
size = opts.size
step = opts.step
noise_level = opts.noise_level
filename = opts.filename
tone = numpy.arange(0, 2*numpy.pi, step)
chunk_size = len(tone)
rep = size // chunk_size
fout = open(filename, "wb")
for i in xrange(rep):
    noise = numpy.random.normal(0.0, noise_level, chunk_size)
    for d in tone + noise:
        fout.write(struct.pack("d", d))
fout.close()
