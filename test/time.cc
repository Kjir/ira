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
        ("input,i", po::value< std::string >(), "Input file where to get the signal")
        ("save,s", po::value< std::string >(), "File where to save the signal")
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
    if( var_map.count("input") ) {
        boost::filesystem::path fp( var_map["input"].as< std::string >() );
        in = new boost::filesystem::ifstream(fp);
    }

    boost::filesystem::path order_fp( "order.dat" );
    boost::filesystem::ofstream order_f( order_fp );
    for( int o = 10; o < 27; o++ ) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        siglen = boost::numeric_cast<int>( pow(2, o) );
        result = computeFFT(var_map, in, hint, o, 1, scaling, pscaling);
        write_result(var_map, result, siglen);
        ippFree(result);
        result = NULL;
        gettimeofday(&end, NULL);
        double e = ((end.tv_sec - start.tv_sec) * 1000) + (end.tv_usec /1000 - start.tv_usec / 1000);
        order_f << o << '\t' << e << std::endl;
    }
    order_f.close();

    boost::filesystem::path orderint_fp( "order_int.dat" );
    boost::filesystem::ofstream orderint_f( orderint_fp );
    (*in).seekg(0);
    for( int o = 10; o < 27; o++ ) {
        struct timeval start, end;
        /* Time before the FFT */
        gettimeofday(&start, NULL);

        siglen = boost::numeric_cast<int>( pow(2, o) );
        result = computeFFT(var_map, in, hint, o, 30, scaling, pscaling);
        write_result(var_map, result, siglen);
        ippFree(result);
        result = NULL;

        /* Time after the FFT */
        gettimeofday(&end, NULL);
        double e = ((end.tv_sec - start.tv_sec) * 1000) + (end.tv_usec /1000 - start.tv_usec / 1000);
        orderint_f << o << '\t' << e << std::endl;
    }
    orderint_f.close();

    boost::filesystem::path int_fp( "integration.dat" );
    boost::filesystem::ofstream int_f( int_fp );
    (*in).seekg(0);
    siglen = boost::numeric_cast<int>( pow(2, 15) );
    for( int i = 20; i < 3000; i += 30 ) {
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

    IppStatus s = ippSetNumThreads(1);
    if( s != ippStsNoErr ) {
        std::cerr << "Error thread: " << ippGetStatusString(s) << std::endl;
    }
    boost::filesystem::path nomulti_fp( "nomulti.dat" );
    boost::filesystem::ofstream nomulti_f( nomulti_fp );
    for( int o = 10; o < 27; o++ ) {
        struct timeval start, end;
        gettimeofday(&start, NULL);

        siglen = boost::numeric_cast<int>( pow(2, o) );
        result = computeFFT(var_map, in, hint, o, 1, scaling, pscaling);
        write_result(var_map, result, siglen);
        ippFree(result);
        result = NULL;

        gettimeofday(&end, NULL);
        double e = ((end.tv_sec - start.tv_sec) * 1000) + (end.tv_usec /1000 - start.tv_usec / 1000);
        nomulti_f << o << '\t' << e << std::endl;
    }
    nomulti_f.close();

    s = ippSetNumThreads(5);
    if( s != ippStsNoErr ) {
        std::cerr << "Error thread2: " << ippGetStatusString(s) << std::endl;
    }
    boost::filesystem::path multi5_fp( "multi5.dat" );
    boost::filesystem::ofstream multi5_f( multi5_fp );
    for( int o = 10; o < 27; o++ ) {
        struct timeval start, end;
        gettimeofday(&start, NULL);

        siglen = boost::numeric_cast<int>( pow(2, o) );
        result = computeFFT(var_map, in, hint, o, 1, scaling, pscaling);
        write_result(var_map, result, siglen);
        ippFree(result);
        result = NULL;

        gettimeofday(&end, NULL);
        double e = ((end.tv_sec - start.tv_sec) * 1000) + (end.tv_usec /1000 - start.tv_usec / 1000);
        multi5_f << o << '\t' << e << std::endl;
    }
    multi5_f.close();

    if( in != &std::cin ) {
        delete in;
    }


    return 0;
}
