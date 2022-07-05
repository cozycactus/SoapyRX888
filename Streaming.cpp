#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Formats.hpp>

#include <string>
#include "SoapyRX888.hpp"



/*******************************************************************
 * Stream API
 ******************************************************************/

std::vector<std::string> SoapyRX888::getStreamFormats(const int direction, const size_t channel) const {
    std::vector<std::string> formats;

    formats.push_back(SOAPY_SDR_S16);

    return formats;
}

std::string SoapyRX888::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const {
    //check that direction is SOAPY_SDR_RX
     if (direction != SOAPY_SDR_RX) {
         throw std::runtime_error("RX888 is RX only, use SOAPY_SDR_RX");
     }

     fullScale = 32767;
     return SOAPY_SDR_S16;
}