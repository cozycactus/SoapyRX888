#include <SoapySDR/Device.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Formats.hpp>

#include <string>
#include "SoapyRX888.hpp"
#if 0
/***********************************************************************
 * Device interface
 **********************************************************************/
class MyDevice : public SoapySDR::Device
{
    //Implement constructor with device specific arguments...

    //Implement all applicable virtual methods from SoapySDR::Device
};

/***********************************************************************
 * Find available devices
 **********************************************************************/
SoapySDR::KwargsList findMyDevice(const SoapySDR::Kwargs &args)
{
    //locate the device on the system...
    //return a list of 0, 1, or more argument maps that each identify a device
}

/***********************************************************************
 * Make device instance
 **********************************************************************/
SoapySDR::Device *makeMyDevice(const SoapySDR::Kwargs &args)
{
    //create an instance of the device object given the args
    //here we will translate args into something used in the constructor
    return new MyDevice(...);
}

/***********************************************************************
 * Registration
 **********************************************************************/
static SoapySDR::Registry registerMyDevice("my_device", &findMyDevice, &makeMyDevice, SOAPY_SDR_ABI_VERSION);

#endif

/*******************************************************************
 * Identification API
 ******************************************************************/

std::string SoapyRX888::getDriverKey(void) const
{
    return "RX888";
}

std::string SoapyRX888::getHardwareKey(void) const
{
    return "RX888";
}

/*******************************************************************
 * Channels API
 ******************************************************************/

size_t SoapyRX888::getNumChannels(const int dir) const
{
    return (dir == SOAPY_SDR_RX) ? 1 : 0;
}

bool SoapyRX888::getFullDuplex(const int direction, const size_t channel) const
{
    return false;
}

/*******************************************************************
 * Stream API
 ******************************************************************/

std::vector<std::string> SoapyRX888::getStreamFormats(const int direction, const size_t channel) const {
    std::vector<std::string> formats;

    formats.push_back(SOAPY_SDR_S16);

    return formats;
}
