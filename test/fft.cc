#include <iostream>
#include <ipp.h>

#define ORDER 8
#define SIGLEN 256

int main( void ) {
    Ipp16s signal[SIGLEN];
    IppStatus status;
    IppsFFTSpec_R_16s *FFTSpec;
    int bufsize;
    Ipp8u *buffer;

    std::cin.read( (char *)signal, sizeof signal );
    std::cerr << "Printing first 10 elements: \n";
    for(int i = 0; i < 10; i++) {
        std::cerr << i << ": " << signal[i] << "\n";
    }


    ippStaticInit();
    status = ippsFFTInitAlloc_R_16s(&FFTSpec, ORDER, IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate);
    if( status != ippStsNoErr ) {
        std::cerr << "IPP Error in InitAlloc: " << ippGetStatusString(status) << "\n";
        return -1;
    }
    status = ippsFFTGetBufSize_R_16s( FFTSpec, &bufsize );
    if( status != ippStsNoErr ) {
        std::cerr << "IPP Error in FFTGetBufSize: " << ippGetStatusString(status) << "\n";
        return -2;
    }
    buffer = ippsMalloc_8u(bufsize);
    if( buffer == NULL ) {
        std::cerr << "Not enough memory\n";
        return -3;
    }
    status = ippsFFTFwd_RToPack_16s_ISfs(signal, FFTSpec, 3, buffer);
    if( status != ippStsNoErr ) {
        std::cerr << "IPP Error in FFTFwd: " << ippGetStatusString(status) << "\n";
        return -4;
    }
    ippsFFTFree_R_16s(FFTSpec);
    FFTSpec = NULL;
    ippsFree( buffer );
    buffer = NULL;

    std::cerr << "Printing first 10 elements after FFT: \n";
    for(int i = 0; i < 10; i++) {
        std::cerr << i << ": " << signal[i] << "\n";
    }
    std::cout.write( (char *)signal, sizeof signal);

    return 0;
}
