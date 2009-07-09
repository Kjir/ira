#include <ipp.h>
#include <iostream>
#include <boost/program_options.hpp>
Ipp16s *computeFFT(boost::program_options::variables_map &, std::istream *, IppHintAlgorithm, int, int, int, int);
