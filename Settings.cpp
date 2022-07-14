
#include "SoapyRX888.hpp"
#include <SoapySDR/Types.hpp>




SoapyRX888::SoapyRX888(const SoapySDR::Kwargs &args):
    deviceId(-1),
    dev(nullptr),
    rxFormat(RX888_RX_FORMAT_INT16),
    sampleRate(64000000)
{
    if (args.count("label") != 0) SoapySDR_logf(SOAPY_SDR_INFO, "Opening %s...", args.at("label").c_str());

    //if a serial is not present, then findRTLSDR had zero devices enumerated
    if (args.count("serial") == 0) throw std::runtime_error("No RX888 devices found!");

    const auto serial = args.at("serial");
    deviceId = rx888_get_index_by_serial(serial.c_str());
    if (deviceId < 0) throw std::runtime_error("rx888_get_index_by_serial("+serial+") - " + std::to_string(deviceId));

    
    
    SoapySDR_logf(SOAPY_SDR_DEBUG, "RTL-SDR opening device %d", deviceId);
    if (rx888_open(&dev, deviceId) != 0) {
        throw std::runtime_error("Unable to open RX888 device");
    }

    
}


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