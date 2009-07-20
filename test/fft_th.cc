#include "fft.h"
#include <stdlib.h>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/thread.hpp>

struct buffer {
    boost::mutex mut;
    boost::condition_variable dready;
    bool data_ready = false;
    Ipp16s *sig;
};

Ipp16s *malloc_16s( int length ) {
    Ipp16s *mem = ippsMalloc_16s(length);
    if( mem == NULL ) {
        std::cerr << "Not enough memory" << std::endl;
        exit(3);
    }
}

void fetch_data(std::istream in, int siglen, int numint, struct buffer *buf) {
    int i = 0;
    while( (*in).good() && i < numint ) {
        boost::unique_lock<boost::mutex> lock(buf->mut);
        while( buf->data_ready ) {
            buf->dready.wait(lock);
        }

        (*in).read( (char *)buf->sig, sizeof(*(buf->sig)) * siglen );
        buf->cond.notify_one();
    }
}

Ipp16s *computeFFT(boost::program_options::variables_map &var_map, std::istream *in, IppHintAlgorithm hint, int order, int nint, int scaling, int pscaling) {
    Ipp16s *tmpdst, *result;
    IppStatus status;
    Ipp8u *buffer;
    IppsFFTSpec_R_16s *FFTSpec;
    int bufsize;
    struct buffer buf;


    int siglen = boost::numeric_cast<int>( pow(2, order) );

    result = malloc_16s(siglen);
    ippsSet_16s(0, result, siglen);
    buf.sig = malloc_16s(siglen);
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

    boost::thread th(fetch_data, siglen, nint, &buf);

    for( int i = 0; i < nint; i++ ) {

        status = ippsFFTFwd_RToPack_16s_Sfs(buf.sig, tmpdst, FFTSpec, scaling, buffer);
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

    ippFree(buf.sig);
    ippFree(tmpdst);
    buf.sig = NULL;
    tmpdst = NULL;
    ippsFFTFree_R_16s(FFTSpec);
    FFTSpec = NULL;
    ippsFree( buffer );
    buffer = NULL;

    return result;
}
