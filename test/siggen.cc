#include <iostream>
#include <ipp.h>

#define SIGLEN 256

int main( void ) {
    Ipp16s signal[SIGLEN] = { -1 };
    IppStatus status;
    float phase = 1.4;
    ippStaticInit();
    status = ippsTone_Direct_16s(signal, SIGLEN, 10000, 0.2, &phase, ippAlgHintAccurate);
    if(status != ippStsNoErr) {
        std::cerr << "IPP Error in Tone Direct: " << ippGetStatusString(status) << "\n";
    }
    std::cout.write((char *)signal, sizeof signal);
    return 0;
}
