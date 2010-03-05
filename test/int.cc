#include <iostream>
#include <ipp.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <sys/time.h>
#include "fft.h"
#include "common.h"

int main( int argc, char **argv ) {
    namespace po = boost::program_options;
    po::options_description desc("Program options");
    po::variables_map var_map;
    Ipp16s *result;
    IppStatus status;
    IppHintAlgorithm hint;
    int scaling, pscaling = 13, siglen = 0;

    desc.add_options()
        ("help", "Print this help message")
        ("fast", "Use fast algorithm instead of accurate one")
        ("scaling,t", po::value<int>(&scaling)->default_value(0), "Scaling value. The output will be multiplied by 2^-scaling")
        ("file,f", po::value< std::string >(), "Input file where to get the signal")
        ("outfile,o", po::value< std::string >(), "File where to save the signal")
        ("power-spectrum,p", po::value<int>(&pscaling), "Convert the complex numbers into the power spectrum with the defined scaling")
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

    std::istream *in = &std::cin;
    if( var_map.count("file") ) {
        boost::filesystem::path fp( var_map["file"].as< std::string >() );
        in = new boost::filesystem::ifstream(fp);
    }

    boost::filesystem::path int_fp( "integration.dat" );
    boost::filesystem::ofstream int_f( int_fp );
    siglen = boost::numeric_cast<int>( pow(2, 15) );
    for( int i = 1; i < 13000; i += 1000 ) {
        (*in).seekg(0);
        struct timeval start, end;
        gettimeofday(&start, NULL);

        result = computeFFT(var_map, in, hint, 15, i, scaling, pscaling);
        write_result(var_map, result, siglen);
        ippFree(result);
        result = NULL;

        gettimeofday(&end, NULL);
        double e = ((end.tv_sec - start.tv_sec) * 1000) + (end.tv_usec /1000 - start.tv_usec / 1000);
        int_f << i << '\t' << e << std::endl;
    }
    int_f.close();

    if( in != &std::cin ) {
        delete in;
    }


    return 0;
}
