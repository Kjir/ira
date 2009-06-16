bin_PROGRAMS = fft siggen lambda subject fftinv

fft_SOURCES = fft.cc
fft_CPPFLAGS=$(IPP_INCLUDES)
fft_LDFLAGS=$(IPP_LDFLAGS)
fft_LDADD = $(IPP_LIBS)

fftinv_SOURCES = fftinv.cc
fftinv_CPPFLAGS=$(IPP_INCLUDES)
fftinv_LDFLAGS=$(IPP_LDFLAGS)
fftinv_LDADD = $(IPP_LIBS)

siggen_SOURCES = siggen.cc
siggen_CPPFLAGS=$(IPP_INCLUDES) $(BOOST_CPPFLAGS)
siggen_LDFLAGS=$(IPP_LDFLAGS) $(BOOST_LDFLAGS)
siggen_LDADD = $(IPP_LIBS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_FILESYSTEM_LIB)

lambda_SOURCES = lambda.cc
lambda_CPPFLAGS = $(BOOST_CPPFLAGS)
lambda_LDFLAGS = $(BOOST_LDFLAGS)
subject_SOURCES = subject.cc
subject_CPPFLAGS = $(BOOST_CPPFLAGS)
subject_LDFLAGS = $(BOOST_LDFLAGS)
subject_LDADD = $(BOOST_REGEX_LIB)
