
#include "SoapyRX888.hpp"
#include <SoapySDR/Time.hpp>
#include <SoapySDR/Types.hpp>
#include <algorithm>

SoapyRX888::SoapyRX888(const SoapySDR::Kwargs &args):
    deviceId(-1),
    dev(nullptr),
    rxFormat(RX888_RX_FORMAT_INT16),
    sampleRate(64000000),
    numBuffers(DEFAULT_NUM_BUFFERS)
    
{
    if (args.count("label") != 0) SoapySDR_logf(SOAPY_SDR_INFO, "Opening %s...", args.at("label").c_str());

    //if a serial is not present, then findRX888 had zero devices enumerated
    if (args.count("serial") == 0) throw std::runtime_error("No RX888 devices found!");

    const auto serial = args.at("serial");
    deviceId = rx888_get_index_by_serial(serial.c_str());
    if (deviceId < 0) throw std::runtime_error("rx888_get_index_by_serial("+serial+") - " + std::to_string(deviceId));
    
    SoapySDR_logf(SOAPY_SDR_DEBUG, "RX888 opening device %d", deviceId);
    if (rx888_open(&dev, deviceId) != 0) {
        throw std::runtime_error("Unable to open RX888 device");
    }

    
}

SoapyRX888::~SoapyRX888(void)
{
    //cleanup device handles
    rx888_close(dev);
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

SoapySDR::Kwargs SoapyRX888::getHardwareInfo(void) const
{
    //key/value pairs for any useful information
    //this also gets printed in --probe
    SoapySDR::Kwargs args;

    args["origin"] = "https://github.com/cozycactus/SoapyRX888";
    args["index"] = std::to_string(deviceId);

    return args;
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
    (void)direction;
    (void)channel;
    return false;
}

/*******************************************************************
 * Antenna API
 ******************************************************************/

std::vector<std::string> SoapyRX888::listAntennas(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    std::vector<std::string> antennas;
    antennas.push_back("RX");
    return antennas;
}

void SoapyRX888::setAntenna(const int direction, const size_t channel, const std::string &name)
{
    (void)channel;
    (void)name;
    if (direction != SOAPY_SDR_RX)
    {
        throw std::runtime_error("setAntena failed: RX888 only supports RX");
    }
}

std::string SoapyRX888::getAntenna(const int direction, const size_t channel) const
{
    (void)channel;
    (void)direction;
    return "RX";
}

/*******************************************************************
 * Frontend corrections API
 ******************************************************************/

bool SoapyRX888::hasDCOffsetMode(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    return false;
}

bool SoapyRX888::hasFrequencyCorrection(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    return false;
}

/*******************************************************************
 * Gain API
 ******************************************************************/

std::vector<std::string> SoapyRX888::listGains(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    std::vector<std::string> gains;
    gains.push_back("RF");
    return gains;
}

bool SoapyRX888::hasGainMode(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    return false;
}

void SoapyRX888::setGain(const int direction, const size_t channel, const std::string &name, const double value) 
{
    (void)direction;
    (void)channel;
    if (name == "RF")
    {
        rx888_set_hf_attenuation(dev, value);
    }
}

SoapySDR::Range SoapyRX888::getGainRange(const int direction, const size_t channel, const std::string &name) const
{
    (void)direction;
    (void)channel;
    if (name == "RF")
    {
        return SoapySDR::Range(-20.0, 0, 10.0);
    } else {
        return SoapySDR::Range(0, 0);
    }
    return SoapySDR::Range(0, 0);
} 

SoapySDR::ArgInfoList SoapyRX888::getFrequencyArgsInfo(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    SoapySDR::ArgInfoList freqArgs;

    // TODO: frequency arguments

    return freqArgs;
}

/*******************************************************************
 * Sample Rate API
 ******************************************************************/

void SoapyRX888::setSampleRate(const int direction, const size_t channel, const double rate)
{
    (void)direction;
    (void)channel;
    long long ns = SoapySDR::ticksToTimeNs(ticks, sampleRate);
    sampleRate = rate;
    resetBuffer = true;
    SoapySDR_logf(SOAPY_SDR_DEBUG, "Setting sample rate: %d", sampleRate);
    int r = rx888_set_sample_rate(dev, sampleRate);
    if (r == -EINVAL)
    {
        throw std::runtime_error("setSampleRate failed: RX888 does not support this sample rate");
    }
    if (r != 0)
    {
        throw std::runtime_error("setSampleRate failed");
    }
    sampleRate = rx888_get_sample_rate(dev);
    ticks = SoapySDR::timeNsToTicks(ns, sampleRate);
}

double SoapyRX888::getSampleRate(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    return sampleRate;
}

std::vector<double> SoapyRX888::listSampleRates(const int direction, const size_t channel) const
{
    (void)direction;
    (void)channel;
    std::vector<double> results;

    results.push_back(250000);
    results.push_back(500000);
    results.push_back(1000000);
    results.push_back(2000000);
    results.push_back(4000000);
    results.push_back(8000000);
    results.push_back(16000000);
    results.push_back(32000000);
    results.push_back(64000000);
    results.push_back(128000000);
    results.push_back(150000000);

    return results;
}

/*******************************************************************
 * Time API
 ******************************************************************/

std::vector<std::string> SoapyRX888::listTimeSources(void) const
{
    std::vector<std::string> results;

    results.push_back("sw_ticks");

    return results;
}

std::string SoapyRX888::getTimeSource(void) const
{
    return "sw_ticks";
}

bool SoapyRX888::hasHardwareTime(const std::string &what) const
{
    return what == "" || what == "sw_ticks";
}

long long SoapyRX888::getHardwareTime(const std::string &what) const
{
    (void)what;
    return SoapySDR::ticksToTimeNs(ticks, sampleRate);
}

void SoapyRX888::setHardwareTime(const long long timeNs, const std::string &what)
{
    (void)what;
    ticks = SoapySDR::timeNsToTicks(timeNs, sampleRate);
}
