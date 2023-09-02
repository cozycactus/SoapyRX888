#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Formats.h>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Formats.hpp>

#include <SoapySDR/Time.hpp>
#include <cstring>
#include "SoapyRX888.hpp"



/*******************************************************************
 * Stream API
 ******************************************************************/

std::vector<std::string> SoapyRX888::getStreamFormats(const int direction, const size_t channel) const {
    (void)channel; //unused
    (void)direction; //unused
    std::vector<std::string> formats;

    formats.push_back(SOAPY_SDR_CF32);
    formats.push_back(SOAPY_SDR_S16);

    return formats;
}

std::string SoapyRX888::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const {
    (void)channel; //unused
    //check that direction is SOAPY_SDR_RX
     if (direction != SOAPY_SDR_RX) {
         throw std::runtime_error("RX888 is RX only, use SOAPY_SDR_RX");
     }

     fullScale = 32768;
     return SOAPY_SDR_S16;
}

SoapySDR::ArgInfoList SoapyRX888::getStreamArgsInfo(const int direction, const size_t channel) const {
    (void)channel; //unused
    //check that direction is SOAPY_SDR_RX
     if (direction != SOAPY_SDR_RX) {
         throw std::runtime_error("RX888 is RX only, use SOAPY_SDR_RX");
     }

    SoapySDR::ArgInfoList streamArgs;

    SoapySDR::ArgInfo bufflenArg;
    bufflenArg.key = "bufflen";
    bufflenArg.value = std::to_string(DEFAULT_BUFFER_LENGTH);
    bufflenArg.name = "Buffer Size";
    bufflenArg.description = "Number of bytes per buffer, multiples of 512 only.";
    bufflenArg.units = "bytes";
    bufflenArg.type = SoapySDR::ArgInfo::INT;

    streamArgs.push_back(bufflenArg);

    SoapySDR::ArgInfo buffersArg;
    buffersArg.key = "buffers";
    buffersArg.value = std::to_string(DEFAULT_NUM_BUFFERS);
    buffersArg.name = "Ring buffers";
    buffersArg.description = "Number of buffers in the ring.";
    buffersArg.units = "buffers";
    buffersArg.type = SoapySDR::ArgInfo::INT;

    streamArgs.push_back(buffersArg);

    SoapySDR::ArgInfo asyncbuffsArg;
    asyncbuffsArg.key = "asyncBuffs";
    asyncbuffsArg.value = "0";
    asyncbuffsArg.name = "Async buffers";
    asyncbuffsArg.description = "Number of async usb buffers (advanced).";
    asyncbuffsArg.units = "buffers";
    asyncbuffsArg.type = SoapySDR::ArgInfo::INT;

    streamArgs.push_back(asyncbuffsArg);

    return streamArgs;
}

/*******************************************************************
 * Async thread work
 ******************************************************************/

static void _rx_callback(unsigned char *buf, uint32_t len, void *ctx)
{
    //printf("_rx_callback\n");
    SoapyRX888 *self = static_cast<SoapyRX888*>(ctx);
    self->rx_callback(buf, len);
}

void SoapyRX888::rx_async_operation(void)
{
    //printf("rx_async_operation\n");
    rx888_read_async(dev, &_rx_callback, this, asyncBuffs, bufferLength);
    //printf("rx_async_operation done!\n");
}

void SoapyRX888::rx_callback(unsigned char *buf, uint32_t len)
{
    //printf("_rx_callback %d _buf_head=%d, numBuffers=%d\n", len, _buf_head, _buf_tail);

    // atomically add len to ticks but return the previous value
    unsigned long long tick = ticks.fetch_add(len);

    //overflow condition: the caller is not reading fast enough
    if (_buf_count == numBuffers)
    {
        _overflowEvent = true;
        return;
    }

    //copy into the buffer queue
    auto &buff = _buffs[_buf_tail];
    buff.tick = tick;
    buff.data.resize(len);
    std::memcpy(buff.data.data(), buf, len);

    //increment the tail pointer
    _buf_tail = (_buf_tail + 1) % numBuffers;

    //increment buffers available under lock
    //to avoid race in acquireReadBuffer wait
    {
    std::lock_guard<std::mutex> lock(_buf_mutex);
    _buf_count++;

    }

    //notify readStream()
    _buf_cond.notify_one();
}

/*******************************************************************
 * Stream API
 ******************************************************************/

SoapySDR::Stream *SoapyRX888::setupStream(
        const int direction,
        const std::string &format,
        const std::vector<size_t> &channels,
        const SoapySDR::Kwargs &args)
{
    if (direction != SOAPY_SDR_RX)
    {
        throw std::runtime_error("RX888 is RX only, use SOAPY_SDR_RX");
    }

    //check the channel configuration
    if (channels.size() > 1 or (channels.size() > 0 and channels.at(0) != 0))
    {
        throw std::runtime_error("setupStream invalid channel selection");
    }

    //check the format
    if (format == SOAPY_SDR_S16)
    {
        SoapySDR_log(SOAPY_SDR_INFO, "Using format S16.");
        rxFormat = RX888_RX_FORMAT_INT16;
    }
    else if (format == SOAPY_SDR_CF32)
    {
        SoapySDR_log(SOAPY_SDR_INFO, "Using format CF32 with imaginary component set to zero.");
        rxFormat = RX888_RX_FORMAT_FLOAT32;
    }
    else
    {
        throw std::runtime_error(
                "setupStream invalid format '" + format
                        + "' -- Only S16 and CF32 with imag=0 are supported by SoapyRX888 module.");
    }

    bufferLength = DEFAULT_BUFFER_LENGTH;
    if (args.count("bufflen") != 0)
    {
        try
        {
            int bufferLength_in = std::stoi(args.at("bufflen"));
            if (bufferLength_in > 0)
            {
                bufferLength = bufferLength_in;
            }
        }
        catch (const std::invalid_argument &){}
    }
    SoapySDR_logf(SOAPY_SDR_DEBUG, "RX888 Using buffer length %d", bufferLength);

    numBuffers = DEFAULT_NUM_BUFFERS;
    if (args.count("buffers") != 0)
    {
        try
        {
            int numBuffers_in = std::stoi(args.at("buffers"));
            if (numBuffers_in > 0)
            {
                numBuffers = numBuffers_in;
            }
        }
        catch (const std::invalid_argument &){}
    }
    SoapySDR_logf(SOAPY_SDR_DEBUG, "RX888 Using %d buffers", numBuffers);

    asyncBuffs = 0;
    if (args.count("asyncBuffs") != 0)
    {
        try
        {
            int asyncBuffs_in = std::stoi(args.at("asyncBuffs"));
            if (asyncBuffs_in > 0)
            {
                asyncBuffs = asyncBuffs_in;
            }
        }
        catch (const std::invalid_argument &){}
    }

    //clear async fifo counts
    _buf_tail = 0;
    _buf_count = 0;
    _buf_head = 0;

    //allocate buffers
    _buffs.resize(numBuffers);
    for (auto &buff : _buffs) buff.data.reserve(bufferLength);
    for (auto &buff : _buffs) buff.data.resize(bufferLength);

    return (SoapySDR::Stream *) this;
}

void SoapyRX888::closeStream(SoapySDR::Stream *stream)
{
    this->deactivateStream(stream, 0, 0);
    _buffs.clear();
}

size_t SoapyRX888::getStreamMTU(SoapySDR::Stream *stream) const
{
    (void) stream; //unused
    return bufferLength / BYTES_PER_SAMPLE;
}

int SoapyRX888::activateStream(
        SoapySDR::Stream *stream,
        const int flags,
        const long long timeNs,
        const size_t numElems)
{
    (void) stream; //unused
    (void) timeNs; //unused
    (void) numElems; //unused
    if (flags != 0) return SOAPY_SDR_NOT_SUPPORTED;
    resetBuffer = true;
    bufferedElems = 0;

    //start the async thread
    if (not _rx_async_thread.joinable())
    {
        //rx888_reset_buffer(dev);
        _rx_async_thread = std::thread(&SoapyRX888::rx_async_operation, this);
    }

    return 0;
}

int SoapyRX888::deactivateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs)
{
    (void) timeNs; //unused
    (void) stream; //unused
    if (flags != 0) return SOAPY_SDR_NOT_SUPPORTED;
    if (_rx_async_thread.joinable())
    {
        rx888_cancel_async(dev);
        _rx_async_thread.join();
    }
    return 0;
}

int SoapyRX888::readStream(
        SoapySDR::Stream *stream,
        void * const *buffs,
        const size_t numElems,
        int &flags,
        long long &timeNs,
        const long timeoutUs)
{
    //drop remainder buffer on reset
    if (resetBuffer and bufferedElems != 0)
    {
        bufferedElems = 0;
        this->releaseReadBuffer(stream, _currentHandle);
    }

    //this is the user's buffer for channel 0
    void *buff0 = buffs[0];

    //are elements left in the buffer? if not, do a new read.
    if (bufferedElems == 0)
    {
        int ret = this->acquireReadBuffer(stream, _currentHandle, (const void **)&_currentBuff, flags, timeNs, timeoutUs);
        if (ret < 0) return ret;
        bufferedElems = ret;
    }

    //otherwise just update return time to the current tick count
    else
    {
        flags |= SOAPY_SDR_HAS_TIME;
        timeNs = SoapySDR::ticksToTimeNs(bufTicks, sampleRate);
    }

    size_t returnedElems = std::min(bufferedElems, numElems);

    //convert into user's buff0
    if (rxFormat == RX888_RX_FORMAT_INT16)
    {
        int16_t *itarget = reinterpret_cast<int16_t*>(buff0);
        for (size_t i = 0; i < returnedElems; i++)
        {
        itarget[i] = *((int16_t*) &_currentBuff[2 * i]);
        }
    }
    else if (rxFormat == RX888_RX_FORMAT_FLOAT32)
    {
        float *ftarget = reinterpret_cast<float*>(buff0);
        for (size_t i = 0; i < returnedElems; i++)
        {
        int16_t val = *((int16_t*) &_currentBuff[2 * i]);
        ftarget[i * 2] = float(val) / 32768.0f;   // scale int16_t to [-1, 1] range.
        ftarget[i * 2 + 1] = 0.0f; // imaginary part is zero
        }
    }
    //bump variables for next call into readStream
    bufferedElems -= returnedElems;
    _currentBuff += returnedElems * 2; // Each int16_t sample consists of 2 int8_t bytes
    bufTicks += returnedElems; //for the next call to readStream if there is a remainder

    //return number of elements written to buff0
    if (bufferedElems != 0) flags |= SOAPY_SDR_MORE_FRAGMENTS;
    else this->releaseReadBuffer(stream, _currentHandle);
    return returnedElems;
}

/*******************************************************************
 * Direct buffer access API
 ******************************************************************/

size_t SoapyRX888::getNumDirectAccessBuffers(SoapySDR::Stream *stream)
{
    (void)stream; //unused
    return _buffs.size();
}

int SoapyRX888::getDirectAccessBufferAddrs(SoapySDR::Stream *stream, const size_t handle, void **buffs)
{
    (void)stream; //unused
    buffs[0] = static_cast<void*>(_buffs[handle].data.data());
    return 0;
}

int SoapyRX888::acquireReadBuffer(
    SoapySDR::Stream *stream,
    size_t &handle,
    const void **buffs,
    int &flags,
    long long &timeNs,
    const long timeoutUs)
{
    (void)stream; //unused
    //reset is issued by various settings
    //to drain old data out of the queue
    if (resetBuffer)
    {
        //drain all buffers from the fifo
        _buf_head = (_buf_head + _buf_count.exchange(0)) % numBuffers;
        resetBuffer = false;
        _overflowEvent = false;
    }

    //handle overflow from the rx callback thread
    if (_overflowEvent)
    {
        //drain the old buffers from the fifo
        _buf_head = (_buf_head + _buf_count.exchange(0)) % numBuffers;
        _overflowEvent = false;
        SoapySDR::log(SOAPY_SDR_SSI, "O");
        return SOAPY_SDR_OVERFLOW;
    }

    //wait for a buffer to become available
    if (_buf_count == 0)
    {
        std::unique_lock <std::mutex> lock(_buf_mutex);
        _buf_cond.wait_for(lock, std::chrono::microseconds(timeoutUs), [this]{return _buf_count != 0;});
        if (_buf_count == 0) return SOAPY_SDR_TIMEOUT;
    }

    //extract handle and buffer
    handle = _buf_head;
    _buf_head = (_buf_head + 1) % numBuffers;
    bufTicks = _buffs[handle].tick;
    timeNs = SoapySDR::ticksToTimeNs(_buffs[handle].tick, sampleRate);
    buffs[0] = (void *)_buffs[handle].data.data();
    flags = SOAPY_SDR_HAS_TIME;

    //return number available
    return _buffs[handle].data.size() / BYTES_PER_SAMPLE;
}

void SoapyRX888::releaseReadBuffer(
    SoapySDR::Stream *stream,
    const size_t handle)
{
    (void)stream; //unused
    (void)handle; //unused
    //TODO this wont handle out of order releases
    _buf_count--;
}
