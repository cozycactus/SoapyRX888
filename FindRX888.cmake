
# - Try to find rx888 - the device driver
# Once done this will define
#  RX888_FOUND - System has rx888
#  RX888_LIBRARIES - The rx888 libraries
#  RX888_INCLUDE_DIRS - The rx888 include directories
#  RX888_LIB_DIRS - The rx888 library directories

if(NOT RX888_FOUND)

    find_package(PkgConfig)
    pkg_check_modules (RX888_PKG librx888)
    set(RX888_DEFINITIONS ${PC_RX888_CFLAGS_OTHER})

    find_path(RX888_INCLUDE_DIR
                NAMES librx888.h
                HINTS ${RX888_PKG_INCLUDE_DIRS} $ENV{RX888_DIR}/include
                PATHS /usr/local/include /usr/include /opt/include /opt/local/include)

    find_library(RX888_LIBRARY
                NAMES rx888
                HINTS ${RX888_PKG_LIBRARY_DIRS} $ENV{RX888_DIR}/include
                PATHS /usr/local/lib /usr/lib /opt/lib /opt/local/lib)

    set(RX888_LIBRARIES ${RX888_LIBRARY} )
    set(RX888_INCLUDE_DIRS ${RX888_INCLUDE_DIR} )

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set LibRX888_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(RX888  DEFAULT_MSG
                                      RX888_LIBRARY RX888_INCLUDE_DIR)

    mark_as_advanced(RX888_INCLUDE_DIR RX888_LIBRARY)

endif(NOT RX888_FOUND)

