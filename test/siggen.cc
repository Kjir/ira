#include <iostream>
#include <ipp.h>
#include <boost/program_options.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <cmath>

int main( int argc, char **argv ) {
    namespace po = boost::program_options;
    po::options_description desc("Program options");
    po::variables_map var_map;
    Ipp16s *signal;
    IppStatus status;
    IppHintAlgorithm hint;
    float phase, rfreq;
    int siglen;
    Ipp16s magn;

    desc.add_options()
        ("help", "Print this help message")
        ("fast", "Use fast algorithm instead of accurate one")
        ("phase,p", po::value<float>(&phase)->default_value(0.0), "The starting phase of the signal")
        ("magnitude,m", po::value<Ipp16s>(&magn)->default_value(10000), "The maximum magnitude of the signal")
        ("frequency,f", po::value<float>(&rfreq)->default_value(0.2), "Frequency of the signal, relative to sampling frequency. Value between [0.0, 0.5) for reals, [0.0, 1.0) for complexes")
        ("length,l", po::value<int>(&siglen)->default_value(256), "Length of the signal to be generated")
        ("output,o", po::value< std::string >(), "File where to save the signal")
        ;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), var_map);
    po::notify(var_map);

    /*
     * Print help
     * */
    if( var_map.count("help") ) {
        std::cout << desc << "\n";
        return 0;
    }

    ippStaticInit();
    /*
     * Choose between fast or accurate algorithm
     * */
    if( var_map.count("fast") ) {
        hint = ippAlgHintFast;
    } else {
        hint = ippAlgHintAccurate;
    }
    /*
     * Allocate space for the signal
     * */
    signal = ippsMalloc_16s(siglen);
    if( signal == NULL ) {
        std::cerr << "Not enough memory" << std::endl;
        return -1;
    }
    /*
     * Generate a tone
     * */
    status = ippsTone_Direct_16s(signal, siglen, magn, rfreq, &phase, hint);
    if(status != ippStsNoErr) {
        std::cerr << "IPP Error in Tone Direct: " << ippGetStatusString(status) << '\n';
        return -2;
    }
    /*
     * Output the signal, either to file or to stdout
     * */
    if( var_map.count("output") ) {
        boost::filesystem::path fp( var_map["output"].as< std::string >() );
        boost::filesystem::ofstream file( fp );
        file.write((char *)signal, sizeof(*signal) * siglen);
        file.close();
    } else {
        std::cout.write((char *)signal, sizeof(*signal) * siglen);
    }
    ippFree(signal);
    signal = NULL;
    return 0;
}
