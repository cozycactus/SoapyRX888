
#pragma once

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>

//#include <cstdint>
#include <_types/_uint32_t.h>
#include <librx888.h>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

typedef enum rx888RXFormat
{
    RX888_RX_FORMAT_FLOAT32, RX888_RX_FORMAT_INT16, RX888_RX_FORMAT_INT8
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

    void rx_async_operation(void);
    void rx_callback(unsigned char *buf, uint32_t len);

    std::mutex _buf_mutex;
    std::condition_variable _buf_cond;

    std::vector<Buffer> _buffs;
    size_t	_buf_head;
    size_t	_buf_tail;
    std::atomic<size_t>	_buf_count;

    std::atomic<bool> _overflowEvent;
    
};