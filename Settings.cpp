
#include "SoapyRX888.hpp"
#include <SoapySDR/Types.hpp>




SoapyRX888::SoapyRX888(const SoapySDR::Kwargs &args):

{
    
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