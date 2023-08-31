########################################################################
# Project setup -- only needed if device support is a stand-alone build
# We recommend that the support module be built in-tree with the driver.
########################################################################
cmake_minimum_required(VERSION 3.10)
project(SoapyRX888 CXX)


#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

########################################################################
# Header and library resources needed to communicate with the device.
# These may be found within the build tree or in an external project.
########################################################################
set(MY_DEVICE_INCLUDE_DIRS ...)
set(MY_DEVICE_LIBRARIES ...)

include(FeatureSummary)
include(CMakeDependentOption)

########################################################################
# build the module
########################################################################
find_package(SoapySDR CONFIG)

if (NOT SoapySDR_FOUND)
    message(WARNING "SoapySDR development files not found - skipping support")
    return()
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT ENABLE_INTERNAL_RX888)
    find_package(RX888)
endif()
cmake_dependent_option(ENABLE_INTERNAL_RX888 "Enable RX888 support" ON "NOT RX888_FOUND" OFF)
add_feature_info(RX888 ENABLE_INTERNAL_RX888 "Support for RX888 devices")

if (ENABLE_INTERNAL_RX888)
    message(STATUS "RX888 development files not found. Using submodule.")

    if (EXISTS ${PROJECT_SOURCE_DIR}/librx888/CMakeLists.txt)
        add_subdirectory(librx888)
        set(RX888_FOUND TRUE)
        set(RX888_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/librx888/include)
        set(RX888_LIBRARIES rx888)
    else ()
        message(STATUS "RX888 submodule not found. Cloning.")
        execute_process(
            COMMAND git submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE retcode
        )
        if (NOT retcode EQUAL 0)
            message(FATAL_ERROR "Failed to clone RX888 submodule. Please manually clone with: git submodule update --init --recursive")
        endif ()
    endif()
endif()
message(STATUS "RX888_INCLUDE_DIRS - ${RX888_INCLUDE_DIRS}")
message(STATUS "RX888_LIBRARIES - ${RX888_LIBRARIES}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${RX888_INCLUDE_DIRS})

# Test for Atomics
include(CheckAtomic)
if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB OR NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
    set(ATOMIC_LIBS "atomic")
endif()

#enable c++11 features
if(CMAKE_COMPILER_IS_GNUCXX)

    #C++11 is a required language feature for this project
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_STD_CXX11)
    if(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()

    #Thread support enabled (not the same as -lpthread)
    list(APPEND RX888_LIBRARIES -pthread)

    #disable warnings for unused parameters
    add_definitions(-Wno-unused-parameter)

endif(CMAKE_COMPILER_IS_GNUCXX)

if (APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wc++11-extensions")
endif(APPLE)


SOAPY_SDR_MODULE_UTIL(
    TARGET rx888Support
    SOURCES
        SoapyRX888.hpp        
        Registration.cpp
        Settings.cpp
        Streaming.cpp
    LIBRARIES
        ${RX888_LIBRARIES}
        ${ATOMIC_LIBS}
)
