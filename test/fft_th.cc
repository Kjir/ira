#include "fft.h"
#include <stdlib.h>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>

struct buffer {
    boost::mutex mut;
    boost::condition_variable dready;
    bool data_ready;
    Ipp16s *sig;
    boost::mutex rmut;
    Ipp16s *result;
    boost::mutex endmut;
    bool end;
};

Ipp16s *malloc_16s( int length ) {
    Ipp16s *mem = ippsMalloc_16s(length);
    if( mem == NULL ) {
        std::cerr << "Not enough memory" << std::endl;
        exit(3);
    }
}

void fetch_data(std::istream *in, int siglen, int numint, struct buffer *buf) {
    int i = 0;
    while( i < numint ) {
        boost::unique_lock<boost::mutex> lock(buf->mut);
        while( buf->data_ready ) {
            buf->dready.wait(lock);
        }

        //std::cerr << "Reading: " << sizeof(*(buf->sig)) * siglen << std::endl;
        (*in).read( (char *)buf->sig, sizeof(*(buf->sig)) * siglen );
        buf->data_ready = 1;
        i++;
        buf->dready.notify_one();
    }
    buf->endmut.lock();
    buf->end = 1;
    buf->endmut.unlock();
    buf->dready.notify_all();
}

void fft(bool ps, int siglen, struct buffer *buf, IppsFFTSpec_R_16s *FFTSpec, int scaling, Ipp8u *buffer, int pscaling) {
    IppStatus status;
    Ipp16s *tmpdst;
    tmpdst = malloc_16s(siglen);


    while(1) {
        boost::unique_lock<boost::mutex> lock(buf->mut);
        while( !buf->data_ready ) {
            buf->endmut.lock();
            if( buf->end ) {
                buf->endmut.unlock();
                lock.unlock();
                return;
            }
            buf->endmut.unlock();
            buf->dready.wait(lock);
        }

        status = ippsFFTFwd_RToPack_16s_Sfs(buf->sig, tmpdst, FFTSpec, scaling, buffer);
        if( status != ippStsNoErr ) {
            std::cerr << "IPP Error in FFTFwd: " << ippGetStatusString(status) << "\n";
            lock.unlock();
            exit(4);
        }

        buf->data_ready = 0;
        buf->dready.notify_all();
        lock.unlock();

        if( ps ) {
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
        }
        buf->rmut.lock();
        bool print = false;
        if( buf->result[0] > 0 ) {
            print = true;
        }
        status = ippsAdd_16s_I(tmpdst, buf->result, siglen);
        print = false;
        buf->rmut.unlock();
        if( status != ippStsNoErr ) {
            std::cerr << "IPP Error in Add: " << ippGetStatusString(status) << "\n";
            exit(7);
        }
    }

    ippFree(tmpdst);
    tmpdst = NULL;
}

Ipp16s *computeFFT(boost::program_options::variables_map &var_map, std::istream *in, IppHintAlgorithm hint, int order, int nint, int scaling, int pscaling) {
    Ipp16s *result;
    IppStatus status;
    Ipp8u *buffer;
    IppsFFTSpec_R_16s *FFTSpec;
    int bufsize;
    struct buffer buf;
    buf.end = 0;
    buf.data_ready = 0;
    buf.sig = NULL;
    buf.result = NULL;


    int siglen = boost::numeric_cast<int>( pow(2, order) );

    buf.result = malloc_16s(siglen);
    ippsSet_16s(0, buf.result, siglen);
    buf.sig = malloc_16s(siglen);

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

    boost::thread th(fetch_data, in, siglen, nint, &buf);

    boost::thread fft1(fft, var_map.count("power-spectrum") || nint > 1, siglen, &buf, FFTSpec, scaling, buffer, pscaling);
    boost::thread fft2(fft, var_map.count("power-spectrum") || nint > 1, siglen, &buf, FFTSpec, scaling, buffer, pscaling);
    boost::thread fft3(fft, var_map.count("power-spectrum") || nint > 1, siglen, &buf, FFTSpec, scaling, buffer, pscaling);
    boost::thread fft4(fft, var_map.count("power-spectrum") || nint > 1, siglen, &buf, FFTSpec, scaling, buffer, pscaling);
    th.join();
    fft1.join();
    fft2.join();
    fft3.join();
    fft4.join();

    ippFree(buf.sig);
    buf.sig = NULL;
    ippsFFTFree_R_16s(FFTSpec);
    FFTSpec = NULL;
    ippsFree( buffer );
    buffer = NULL;

    return buf.result;
}
