#include <iostream>
#include <ipp.h>
#include <boost/program_options.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

int main( int argc, char **argv ) {
    namespace po = boost::program_options;
    po::options_description desc("Program options");
    po::variables_map var_map;
    Ipp16s *signal;
    IppStatus status;
    IppHintAlgorithm hint;
    IppsFFTSpec_R_16s *FFTSpec;
    int bufsize, order, siglen, scaling;
    Ipp8u *buffer;

    desc.add_options()
        ("help", "Print this help message")
        ("fast", "Use fast algorithm instead of accurate one")
        ("order,o", po::value<int>(&order)->default_value(8), "Order of the signal. The length of the signal is 2^order")
        ("scaling,t", po::value<int>(&scaling)->default_value(0), "Scaling value. The output will be multiplied by 2^-scaling")
        ("input,i", po::value< std::string >(), "Input file where to get the signal")
        ("save,s", po::value< std::string >(), "File where to save the signal")
        ("real", "Convert the complex numbers into a real rappresentation")
        ;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), var_map);
    po::notify(var_map);

    if( var_map.count("help") ) {
        std::cout << desc << "\n";
        return 0;
    }

    siglen = boost::numeric_cast<int>( pow(2, order) );
    signal = ippsMalloc_16s(siglen);
    if( signal == NULL ) {
	    std::cerr << "Not enough memory" << std::endl;
	    return -3;
    }

    if( var_map.count("input") ) {
	    boost::filesystem::path fp( var_map["input"].as< std::string >() );
	    boost::filesystem::ifstream file( fp );
	    file.read( (char *)signal, sizeof signal );
	    file.close();
    } else {
	    std::cin.read( (char *)signal, sizeof signal );
    }
    std::cerr << "Printing first 10 elements: \n";
    for(int i = 0; i < 10; i++) {
        std::cerr << i << ": " << signal[i] << "\n";
    }


    ippStaticInit();
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

    status = ippsFFTFwd_RToPack_16s_ISfs(signal, FFTSpec, scaling, buffer);
    if( status != ippStsNoErr ) {
        std::cerr << "IPP Error in FFTFwd: " << ippGetStatusString(status) << "\n";
        return -4;
    }
    ippsFFTFree_R_16s(FFTSpec);
    FFTSpec = NULL;
    ippsFree( buffer );
    buffer = NULL;

    if( var_map.count("real") ) {
	    Ipp16sc *vc;
	    vc = ippsMalloc_16sc(siglen);
	    status = ippsConjPack_16sc(signal, vc, siglen*3);
	    if( status != ippStsNoErr ) {
		std::cerr << "IPP Error in ConjPack: " << ippGetStatusString(status) << "\n";
		return -6;
	    }
	    for( int i = 0; i < siglen; i++ ) {
		    Ipp32f pr, pi, tr, ti;
		    tr = vc[i].re;
		    ti = vc[i].im;
		    status = ippsPowx_32f_A11(&tr, (Ipp32f)2, &pr, 1);
		    if( status != ippStsNoErr ) {
			std::cerr << "IPP Error in Powx: " << ippGetStatusString(status) << "\n";
			return -5;
		    }
		    status = ippsPowx_32f_A11(&ti, (Ipp32f)2, &pi, 1);
		    if( status != ippStsNoErr ) {
			std::cerr << "IPP Error in Powx: " << ippGetStatusString(status) << "\n";
			return -5;
		    }
		    signal[i] = pr + pi;
		    std::cerr << "Component exp " << i << ' ' << signal[i] << std::endl;
	    }
	    ippsFree(vc);
	    vc = NULL;
    }

    std::cerr << "Printing first 10 elements after FFT: \n";
    for(int i = 0; i < 10 && i < siglen; i++) {
        std::cerr << i << ": " << signal[i] << "\n";
    }
    if( var_map.count("save") ) {
	    boost::filesystem::path fp( var_map["save"].as< std::string >() );
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
