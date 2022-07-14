


#include "SoapyRX888.hpp"
#include <SoapySDR/Registry.hpp>



/***********************************************************************
 * Find available devices
 **********************************************************************/
SoapySDR::KwargsList findRX888(const SoapySDR::Kwargs &args)
{
    //locate the device on the system...
    //return a list of 0, 1, or more argument maps that each identify a device

    std::vector<SoapySDR::Kwargs> results;

    char manufact[256], product[256], serial[256];

    const size_t this_count = rx888_get_device_count();

    for (size_t i = 0; i < this_count; i++)
    {
        if (rx888_get_device_usb_strings(i, manufact, product, serial) != 0)
        {
            SoapySDR_logf(SOAPY_SDR_ERROR, "rx888_get_device_usb_strings(%zu) failed", i);
            continue;
        }
        SoapySDR_logf(SOAPY_SDR_DEBUG, "\tManufacturer: %s, Product Name: %s, Serial: %s", manufact, product, serial);

        SoapySDR::Kwargs devInfo;
        devInfo["label"] = std::string(rx888_get_device_name(i)) + " :: " + serial;
        devInfo["product"] = product;
        devInfo["serial"] = serial;
        devInfo["manufacturer"] = manufact;
        
        //filtering by serial
        if (args.count("serial") != 0 and args.at("serial") != serial) continue;

        results.push_back(devInfo);
    }

    return results;

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

