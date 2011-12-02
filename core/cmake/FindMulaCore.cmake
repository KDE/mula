# - Try to find the Mula Core library.
#
# Once done this will define
#  MULA_CORE_FOUND - The Mula Core library was found
#  MULA_CORE_INCLUDE_DIR - The Mula include directory
#  MULA_CORE_INCLUDE_DIRS - All include directories required for the Mula Core library
#  MULA_CORE_LIBRARY - The Mula Core library location
#  MULA_CORE_LIBRARIES - The libraries to link against to use Mula Core
#
#  Copyright (C) 2011 Laszlo Papp
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the COPYING-CMAKE-SCRIPTS file from Mula's Source tree.

if (MULA_CORE_LIBRARY AND MULA_CORE_INCLUDE_DIR)
  # Already in cache, be silent
  set(MULA_CORE_FOUND TRUE)
endif (MULA_CORE_LIBRARY AND MULA_CORE_INCLUDE_DIR)

if (MulaCore_FIND_REQUIRED)
    set(_mulaCoreReq "REQUIRED")
endif (MulaCore_FIND_REQUIRED)

set(MULA_VERSION_MAJOR     0                CACHE STRING "Mula Major Version")
set(MULA_VERSION_MINOR     1                CACHE STRING "Mula Minor Version")
set(MULA_VERSION_PATCH     0                CACHE STRING "Mula Patch Version")
set(MULA_VERSION_STRING
    "${MULA_VERSION_MAJOR}.${MULA_VERSION_MINOR}.${MULA_VERSION_PATCH}" CACHE STRING "Mula Version String")
set(MULA_VERSION_NAME      "Alpha Release"  CACHE STRING "Mula Version Name")

find_path(MULA_INSTALL_PREFIX
    NAMES
    include/mula
)

find_package(Qt4 ${_mulaCoreReq})

if(NOT LIB_SUFFIX)
    set(_Init_LIB_SUFFIX "")
    if ("${QT_QTCORE_LIBRARY}" MATCHES lib64)
        set(_Init_LIB_SUFFIX 64)
    endif ("${QT_QTCORE_LIBRARY}" MATCHES lib64)
    if ("${QT_QTCORE_LIBRARY}" MATCHES lib32)
        set(_Init_LIB_SUFFIX 32)
    endif ("${QT_QTCORE_LIBRARY}" MATCHES lib32)

    set(LIB_SUFFIX              ${_Init_LIB_SUFFIX} CACHE STRING "The suffix of the system-wide library path")
endif(NOT LIB_SUFFIX)

if(NOT INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR ${MULA_INSTALL_PREFIX}/include CACHE PATH "The subdirectory relative to the install prefix where header files will be installed.")
endif()
if(NOT LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${MULA_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed.")
endif()
if(NOT SHARE_INSTALL_DIR)
    set(SHARE_INSTALL_DIR ${MULA_INSTALL_PREFIX}/share CACHE PATH "The subdiractory relative to the install prefix where data and other files will be installed.")
endif()

find_path(MULA_CORE_INCLUDE_DIR
    NAMES
    core/singleton.h
    PATHS
    ${INCLUDE_INSTALL_DIR}
    PATH_SUFFIXES
    mula
)

set(MULA_CORE_INCLUDE_DIRS
    ${MULA_CORE_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${EIGEN2_INCLUDE_DIR}
)

find_library(MULA_CORE_LIBRARY
    NAMES
    MulaCore
    PATHS
    ${LIB_INSTALL_DIR}
)

set(MULA_CORE_LIBRARIES
    ${MULA_CORE_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MulaCore DEFAULT_MSG MULA_CORE_LIBRARY MULA_CORE_INCLUDE_DIR)
