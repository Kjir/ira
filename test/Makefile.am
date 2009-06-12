bin_PROGRAMS = fft siggen lambda subject

fft_SOURCES = fft.cc
fft_CPPFLAGS=$(IPP_INCLUDES)
fft_LDFLAGS=$(IPP_LDFLAGS)
fft_LDADD = $(IPP_LIBS)

siggen_SOURCES = siggen.cc
siggen_CPPFLAGS=$(IPP_INCLUDES)
siggen_LDFLAGS=$(IPP_LDFLAGS)
siggen_LDADD = $(IPP_LIBS)

lambda_SOURCES = lambda.cc
lambda_CPPFLAGS = $(BOOST_CPPFLAGS)
lambda_LDFLAGS = $(BOOST_LDFLAGS)
subject_SOURCES = subject.cc
subject_CPPFLAGS = $(BOOST_CPPFLAGS)
subject_LDFLAGS = $(BOOST_LDFLAGS)
subject_LDADD = $(BOOST_REGEX_LIB)

EXTRA_DIST=fft.cc
