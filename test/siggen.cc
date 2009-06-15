#include <iostream>
#include <ipp.h>
#include <boost/program_options.hpp>

#define SIGLEN 256

int main( int argc, char **argv ) {
    namespace po = boost::program_options;
    po::options_description desc("Program options");
    po::variables_map var_map;
    Ipp16s signal[SIGLEN];
    IppStatus status;
    IppHintAlgorithm hint;
    float phase = 1.4;

    desc.add_options()
        ("help", "Print this help message")
        ("fast", "Use fast algorithm instead of accurate one")
        ("phase", po::value<float>(&phase)->default_value(0.0), "The starting phase of the signal")
        ;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), var_map);
    po::notify(var_map);

    if( var_map.count("help") ) {
        std::cout << desc << "\n";
        return 0;
    }

    ippStaticInit();
    if( var_map.count("fast") ) {
        hint = ippAlgHintFast;
    } else {
        hint = ippAlgHintAccurate;
    }
    status = ippsTone_Direct_16s(signal, SIGLEN, 10000, 0.2, &phase, hint);
    if(status != ippStsNoErr) {
        std::cerr << "IPP Error in Tone Direct: " << ippGetStatusString(status) << "\n";
    }
    std::cerr << "Printing first 10 elements: \n";
    for(int i = 0; i < 10; i++) {
        std::cerr << i << ": " << signal[i] << "\n";
    }
    std::cout.write((char *)signal, sizeof signal);
    return 0;
}
