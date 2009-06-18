#!/usr/bin/env python
import sys
import struct
import numpy
from optparse import OptionParser
import pylab

def ps(complex_data):
    return numpy.array([a.real ** 2 + a.imag ** 2 for a in complex_data])

op = OptionParser()
op.add_option("-i", dest="filename", default="signal.dat")
op.add_option("-l", dest="fftl", type="int", default=1024)
op.add_option("-n", dest="integrations", type="int", default=10)
opts, args = op.parse_args(sys.argv[1:])
filename = opts.filename
fftl = opts.fftl
integrations = opts.integrations
fmt = str(fftl) + "d"
fin = open(filename, "rb")
result = numpy.zeros((fftl//2,), dtype=numpy.float)
for i in xrange(integrations):
    buf = fin.read(fftl * 8)
    data = struct.unpack(fmt, buf)
    spectrum = numpy.fft.rfft(data)
    result += ps(spectrum[1:])
pylab.plot(result)
pylab.show()
