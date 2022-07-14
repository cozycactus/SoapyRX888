
#pragma once

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.h>
#include <SoapySDR/Types.h>
#include <SoapySDR/Types.hpp>
#include <cstdint>
#include <librx888.h>

typedef enum rx888RXFormat
{
    RX888_RX_FORMAT_FLOAT32, RX888_RX_FORMAT_INT16, RX888_RX_FORMAT_INT8
} rx888RXFormat;


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


private:

    //device handle
    int deviceId;
    rx888_dev_t *dev;

    //cached settings
    rx888RXFormat rxFormat;
    uint32_t sampleRate;
    
};