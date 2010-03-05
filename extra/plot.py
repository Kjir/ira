#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import struct
import numpy
from optparse import OptionParser
import pylab

op = OptionParser()
op.add_option("-f", dest="filename", default="file.sig")
op.add_option("-l", dest="length", type="int", default=256)
opts, args = op.parse_args(sys.argv[1:])
fmt = "%dh" % opts.length
fin = open(opts.filename, "rb")
buffer = fin.read(2 * opts.length)
data = struct.unpack(fmt, buffer)
pylab.plot(numpy.array(data, dtype="short"))
pylab.show()
