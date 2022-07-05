


#include "SoapyRX888.hpp"
#include <SoapySDR/Registry.hpp>



/***********************************************************************
 * Find available devices
 **********************************************************************/
SoapySDR::KwargsList findRX888(const SoapySDR::Kwargs &args)
{
    //locate the device on the system...
    //return a list of 0, 1, or more argument maps that each identify a device
}


/***********************************************************************
 * Make device instance
 **********************************************************************/
SoapySDR::Device *makeRX888(const SoapySDR::Kwargs &args)
{
    //create an instance of the device object given the args
    //here we will translate args into something used in the constructor
    return new SoapyRX888(args);
}

/***********************************************************************
 * Registration
 **********************************************************************/
static SoapySDR::Registry registerRX888("rx888", &findRX888, &makeRX888, SOAPY_SDR_ABI_VERSION);

