#include <iostream>
#include <ipp.h>
#include <boost/program_options.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sys/time.h>

Ipp16s *malloc_16s( int length ) {
    Ipp16s *mem = ippsMalloc_16s(length);
    if( mem == NULL ) {
        std::cerr << "Not enough memory" << std::endl;
        exit(3);
    }
}

void computeFFT(boost::program_options::variables_map &var_map, std::istream *in, IppHintAlgorithm hint, int order, int nint, int scaling, int pscaling) {
    Ipp16s *signal, *tmpdst, *result;
    IppStatus status;
    Ipp8u *buffer;
    IppsFFTSpec_R_16s *FFTSpec;
    int bufsize;

    int siglen = boost::numeric_cast<int>( pow(2, order) );
    result = malloc_16s(siglen);
    ippsSet_16s(0, result, siglen);
    signal = malloc_16s(siglen);
    tmpdst = malloc_16s(siglen);

    status = ippsFFTInitAlloc_R_16s(&FFTSpec, order, IPP_FFT_NODIV_BY_ANY, hint);
    if( status != ippStsNoErr ) {
        std::cerr << "IPP Error in InitAlloc: " << ippGetStatusString(status) << "\n";
        exit(1);
    }
    status = ippsFFTGetBufSize_R_16s( FFTSpec, &bufsize );
    if( status != ippStsNoErr ) {
        std::cerr << "IPP Error in FFTGetBufSize: " << ippGetStatusString(status) << "\n";
        exit(2);
    }
    buffer = ippsMalloc_8u(bufsize);
    if( buffer == NULL ) {
        std::cerr << "Not enough memory\n";
        exit(3);
    }

    for( int i = 0; i < nint; i++ ) {
        (*in).read( (char *)signal, sizeof(*signal) * siglen );

        status = ippsFFTFwd_RToPack_16s_Sfs(signal, tmpdst, FFTSpec, scaling, buffer);
        if( status != ippStsNoErr ) {
            std::cerr << "IPP Error in FFTFwd: " << ippGetStatusString(status) << "\n";
            exit(4);
        }

        if( var_map.count("power-spectrum")  || nint > 1 ) {
            Ipp16sc *vc, zero = {0, 0};
            vc = ippsMalloc_16sc(siglen);
            if( vc == NULL ) {
                std::cerr << "Not enough memory\n";
                exit(3);
            }
            //Set the vector to zero
            ippsSet_16sc(zero, vc, siglen);
            status = ippsConjPack_16sc(tmpdst, vc, siglen);
            if( status != ippStsNoErr ) {
                std::cerr << "IPP Error in ConjPack: " << ippGetStatusString(status) << "\n";
                exit(5);
            }
            status = ippsPowerSpectr_16sc_Sfs(vc, tmpdst, siglen, pscaling);
            if( status != ippStsNoErr ) {
                std::cerr << "IPP Error in PowerSpectr: " << ippGetStatusString(status) << "\n";
                exit(6);
            }
            ippsFree(vc);
            vc = NULL;
            status = ippsAdd_16s_I(tmpdst, result, siglen);
            if( status != ippStsNoErr ) {
                std::cerr << "IPP Error in Add: " << ippGetStatusString(status) << "\n";
                exit(7);
            }
        } else {
            status = ippsOr_16u_I((Ipp16u *)tmpdst, (Ipp16u *)result, siglen);
            if( status != ippStsNoErr ) {
                std::cerr << "IPP Error in Or: " << ippGetStatusString(status) << "\n";
                exit(8);
            }
        }
    }

    ippFree(signal);
    ippFree(tmpdst);
    signal = NULL;
    tmpdst = NULL;
    ippsFFTFree_R_16s(FFTSpec);
    FFTSpec = NULL;
    ippsFree( buffer );
    buffer = NULL;

    if( var_map.count("save") ) {
        boost::filesystem::path fp( var_map["save"].as< std::string >() );
        boost::filesystem::ofstream file( fp );
        file.write((char *)result, sizeof(*result) * siglen);
        file.close();
    } else {
        std::cout.write((char *)result, sizeof(*result) * siglen);
    }
    ippFree(result);
    result = NULL;
}

int main( int argc, char **argv ) {
    namespace po = boost::program_options;
    po::options_description desc("Program options");
    po::variables_map var_map;
    Ipp16s *result;
    IppStatus status;
    IppHintAlgorithm hint;
    int scaling, pscaling = 13;

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
        computeFFT(var_map, in, hint, o, 1, scaling, pscaling);
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
        gettimeofday(&start, NULL);
        computeFFT(var_map, in, hint, o, 30, scaling, pscaling);
        gettimeofday(&end, NULL);
        double e = ((end.tv_sec - start.tv_sec) * 1000) + (end.tv_usec /1000 - start.tv_usec / 1000);
        orderint_f << o << '\t' << e << std::endl;
    }
    orderint_f.close();

    boost::filesystem::path int_fp( "integration.dat" );
    boost::filesystem::ofstream int_f( int_fp );
    (*in).seekg(0);
    for( int i = 20; i < 3000; i += 30 ) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        computeFFT(var_map, in, hint, 15, i, scaling, pscaling);
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
        computeFFT(var_map, in, hint, o, 1, scaling, pscaling);
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
        computeFFT(var_map, in, hint, o, 1, scaling, pscaling);
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
