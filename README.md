# This is a work in progress

## Dependencies

* SoapySDR - https://github.com/pothosware/SoapySDR/wiki
* librx888 - https://github.com/cozycactus/librx888
* 
# Example module build for SoapySDR device support

Copy this example wholesale either as a new cmake build project,
or copy it into a subdirectory of an existing driver build system.
Customize the CMakeLists.txt for your driver dependencies,
and customize MyDeviceSupport.cpp to make calls into the
low-level device driver for streaming and configuration.
