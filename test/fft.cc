#include <iostream>
#include <ipp.h>
#include <boost/program_options.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

Ipp16s *malloc_16s( int length ) {
    Ipp16s *mem = ippsMalloc_16s(length);
    if( mem == NULL ) {
        std::cerr << "Not enough memory" << std::endl;
        exit(3);
    }
}

int main( int argc, char **argv ) {
    namespace po = boost::program_options;
    po::options_description desc("Program options");
    po::variables_map var_map;
    Ipp16s *signal, *tmpdst, *result;
    IppStatus status;
    IppHintAlgorithm hint;
    IppsFFTSpec_R_16s *FFTSpec;
    int bufsize, order, siglen, scaling, pscaling = 13, nint, iscaling;
    Ipp8u *buffer;

    desc.add_options()
        ("help", "Print this help message")
        ("fast", "Use fast algorithm instead of accurate one")
        ("order,o", po::value<int>(&order)->default_value(8), "Order of the transform. The input and output signal have a 2^order length")
        ("integrate,n", po::value<int>(&nint)->default_value(1), "Number of integrations to be done")
        ("scaling,t", po::value<int>(&scaling)->default_value(0), "Scaling value. The output will be multiplied by 2^-scaling")
        ("input,i", po::value< std::string >(), "Input file where to get the signal")
        ("save,s", po::value< std::string >(), "File where to save the signal")
        ("power-spectrum,p", po::value<int>(&pscaling), "Convert the complex numbers into the power spectrum with the defined scaling")
        ("integration-scaling,a", po::value<int>(&iscaling)->default_value(5), "Scaling to apply during integration")
        ;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), var_map);
    po::notify(var_map);

    if( var_map.count("help") ) {
        std::cout << desc << "\n";
        return 0;
    }

    ippStaticInit();

    siglen = boost::numeric_cast<int>( pow(2, order) );
    signal = malloc_16s(siglen);
    tmpdst = malloc_16s(siglen);
    result = malloc_16s(siglen);
    ippsSet_16s(0, result, siglen);

    if( var_map.count("fast") ) {
        hint = ippAlgHintFast;
    } else {
        hint = ippAlgHintAccurate;
    }
    status = ippsFFTInitAlloc_R_16s(&FFTSpec, order, IPP_FFT_NODIV_BY_ANY, hint);
    if( status != ippStsNoErr ) {
        std::cerr << "IPP Error in InitAlloc: " << ippGetStatusString(status) << "\n";
        return -1;
    }
    status = ippsFFTGetBufSize_R_16s( FFTSpec, &bufsize );
    if( status != ippStsNoErr ) {
        std::cerr << "IPP Error in FFTGetBufSize: " << ippGetStatusString(status) << "\n";
        return -2;
    }
    std::cerr << "Buffer size is: " << bufsize << '\n';
    buffer = ippsMalloc_8u(bufsize);
    if( buffer == NULL ) {
        std::cerr << "Not enough memory\n";
        return -3;
    }

    std::istream *in = &std::cin;
    if( var_map.count("input") ) {
        boost::filesystem::path fp( var_map["input"].as< std::string >() );
        in = new boost::filesystem::ifstream(fp);
    }
    for( int i = 0; i < nint; i++ ) {
        (*in).read( (char *)signal, sizeof(*signal) * siglen );
        std::cerr << "Printing first 10 elements: \n";
        for(int i = 0; i < 10; i++) {
            std::cerr << i << ": " << signal[i] << "\n";
        }

        status = ippsFFTFwd_RToPack_16s_Sfs(signal, tmpdst, FFTSpec, scaling, buffer);
        if( status != ippStsNoErr ) {
            std::cerr << "IPP Error in FFTFwd: " << ippGetStatusString(status) << "\n";
            return -4;
        }

        if( var_map.count("power-spectrum")  || nint > 1 ) {
            std::cerr << "Power" << std::endl;
            Ipp16sc *vc, zero = {0, 0};
            vc = ippsMalloc_16sc(siglen);
            if( vc == NULL ) {
                std::cerr << "Not enough memory\n";
                return -3;
            }
            //Set the vector to zero
            ippsSet_16sc(zero, vc, siglen);
            status = ippsConjPack_16sc(tmpdst, vc, siglen);
            if( status != ippStsNoErr ) {
                std::cerr << "IPP Error in ConjPack: " << ippGetStatusString(status) << "\n";
                return -5;
            }
            status = ippsPowerSpectr_16sc_Sfs(vc, tmpdst, siglen, pscaling);
            if( status != ippStsNoErr ) {
                std::cerr << "IPP Error in PowerSpectr: " << ippGetStatusString(status) << "\n";
                return -6;
            }
            ippsFree(vc);
            vc = NULL;
            /* scaling here seems not correct, should be checked... */
            status = ippsAdd_16s_ISfs(tmpdst, result, siglen, iscaling);
            std::cerr << "Result: " << result[0] << std::endl;
            std::cerr << "tmpdst: " << tmpdst[0] << std::endl;
            if( status != ippStsNoErr ) {
                std::cerr << "IPP Error in Add: " << ippGetStatusString(status) << "\n";
                return -7;
            }
        } else {
            status = ippsOr_16u_I((Ipp16u *)tmpdst, (Ipp16u *)result, siglen);
            if( status != ippStsNoErr ) {
                std::cerr << "IPP Error in Or: " << ippGetStatusString(status) << "\n";
                return -8;
            }
        }
    }

    if( in != &std::cin ) {
        delete in;
    }
    ippsFFTFree_R_16s(FFTSpec);
    FFTSpec = NULL;
    ippsFree( buffer );
    buffer = NULL;

    std::cerr << "Printing first 10 elements after FFT: \n";
    for(int i = 0; i < 10 && i < siglen; i++) {
        std::cerr << i << ": " << result[i] << "\n";
    }
    if( var_map.count("save") ) {
        boost::filesystem::path fp( var_map["save"].as< std::string >() );
        boost::filesystem::ofstream file( fp );
        file.write((char *)result, sizeof(*result) * siglen);
        file.close();
    } else {
        std::cout.write((char *)result, sizeof(*result) * siglen);
    }
    ippFree(signal);
    ippFree(tmpdst);
    ippFree(result);
    signal = NULL;
    tmpdst = NULL;
    result = NULL;

    return 0;
}
