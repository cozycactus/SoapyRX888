
#pragma once

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>

//#include <cstdint>

#include <librx888.h>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

typedef enum rx888RXFormat
{
    RX888_RX_FORMAT_FLOAT32, RX888_RX_FORMAT_INT16
} rx888RXFormat;

#define DEFAULT_BUFFER_LENGTH (1024 * 16 * 8)
#define DEFAULT_NUM_BUFFERS 16
#define BYTES_PER_SAMPLE 2

class SoapyRX888: public SoapySDR::Device
{
public:
    explicit SoapyRX888(const SoapySDR::Kwargs &args);

    ~SoapyRX888(void);

    /*******************************************************************
     * Identification API
     ******************************************************************/

    std::string getDriverKey(void) const;

    std::string getHardwareKey(void) const;

    SoapySDR::Kwargs getHardwareInfo(void) const;

     /*******************************************************************
     * Channels API
     ******************************************************************/
    
    size_t getNumChannels(const int) const;

    bool getFullDuplex(const int direction, const size_t channel) const;

    /*******************************************************************
     * Stream API
     ******************************************************************/
     
    std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

    std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const;

    SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const;

    SoapySDR::Stream *setupStream(const int direction, const std::string &format, const std::vector<size_t> &channels =
            std::vector<size_t>(), const SoapySDR::Kwargs &args = SoapySDR::Kwargs());

    void closeStream(SoapySDR::Stream *stream);

    size_t getStreamMTU(SoapySDR::Stream *stream) const;

    int activateStream(
            SoapySDR::Stream *stream,
            const int flags = 0,
            const long long timeNs = 0,
            const size_t numElems = 0);

    int deactivateStream(SoapySDR::Stream *stream, const int flags = 0, const long long timeNs = 0);

    int readStream(
            SoapySDR::Stream *stream,
            void * const *buffs,
            const size_t numElems,
            int &flags,
            long long &timeNs,
            const long timeoutUs = 100000);
    
    /*******************************************************************
     * Direct buffer access API
     ******************************************************************/

    size_t getNumDirectAccessBuffers(SoapySDR::Stream *stream);

    int getDirectAccessBufferAddrs(
            SoapySDR::Stream *stream,
            const size_t handle,
            void **buffs);

    int acquireReadBuffer(  //acquire a buffer for reading
            SoapySDR::Stream *stream,
            size_t &handle,
            const void **buffs,
            int &flags,
            long long &timeNs,
            const long timeoutUs = 100000);
    
    void releaseReadBuffer(  //release a buffer after reading
            SoapySDR::Stream *stream,
            const size_t handle);
    
    /*******************************************************************
     * Antenna API
     ******************************************************************/
    
    std::vector<std::string> listAntennas(const int direction, const size_t channel) const;

    void setAntenna(const int direction, const size_t channel, const std::string &name);

    std::string getAntenna(const int direction, const size_t channel) const;

    /*******************************************************************
     * Frontend corrections API
     ******************************************************************/
    
    bool hasDCOffsetMode(const int direction, const size_t channel) const;

    bool hasFrequencyCorrection(const int direction, const size_t channel) const;

    //void setFrequencyCorrection(const int direction, const size_t channel, const double value);

    //double getFrequencyCorrection(const int direction, const size_t channel) const;

    /*******************************************************************
     * Gain API
     ******************************************************************/
    
    //std::vector<std::string> listGains(const int direction, const size_t channel) const;

    bool hasGainMode(const int direction, const size_t channel) const;

    //void setGainMode(const int direction, const size_t channel, const bool automatic);

    //bool getGainMode(const int direction, const size_t channel) const;

    //void setGain(const int direction, const size_t channel, const double value);

    //void setGain(const int direction, const size_t channel, const std::string &name, const double value);

    //double getGain(const int direction, const size_t channel) const;

    //SoapySDR::Range getGainRange(const int direction, const size_t channel) const;

    /*******************************************************************
     * Frequency API
     ******************************************************************/
    
    //void setFrequency(const int direction,const size_t channel,const std::string &name,const double frequency,const SoapySDR::Kwargs &args = SoapySDR::Kwargs());
    
    //double getFrequency(const int direction, const size_t channel, const std::string &name) const;

    //std::vector<std::string> listFrequencies(const int direction, const size_t channel) const;

    //SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel, const std::string &name) const;

    SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const;
    
    /*******************************************************************
     * Sample Rate API
     ******************************************************************/
    
    void setSampleRate(const int direction, const size_t channel, const double rate);

    double getSampleRate(const int direction, const size_t channel) const;

    std::vector<double> listSampleRates(const int direction, const size_t channel) const;

    /*******************************************************************
     * Time API
     ******************************************************************/

    std::vector<std::string> listTimeSources(void) const;

    std::string getTimeSource(void) const;

    bool hasHardwareTime(const std::string &what = "") const;

    long long getHardwareTime(const std::string &what = "") const;

    void setHardwareTime(const long long timeNs, const std::string &what = "");





   

private:

    //device handle
    int deviceId;
    rx888_dev_t *dev;

    //cached settings
    rx888RXFormat rxFormat;
    uint32_t sampleRate;
    size_t numBuffers, bufferLength, asyncBuffs;
    std::atomic<long long> ticks;

public:

    struct Buffer
    {
        unsigned long long tick;
        std::vector<signed char> data;
    };

    //async api usage
    std::thread _rx_async_thread;
    void rx_async_operation(void);
    void rx_callback(unsigned char *buf, uint32_t len);

    std::mutex _buf_mutex;
    std::condition_variable _buf_cond;

    std::vector<Buffer> _buffs;
    size_t	_buf_head;
    size_t	_buf_tail;
    std::atomic<size_t>	_buf_count;
    signed char *_currentBuff;
    std::atomic<bool> _overflowEvent;
    size_t _currentHandle;
    size_t bufferedElems;
    long long bufTicks;
    std::atomic<bool> resetBuffer;
    
};