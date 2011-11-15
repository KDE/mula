#################################################################################
# This file is part of the MULA project
# Copyright (c) 2010 Laszlo Papp <lpapp@kde.org>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#/

cmake_minimum_required(VERSION 2.8)
project(MULA)

#Project options, to provide a selective build process
#Note that some of these are mutually exclusive or just
#do not make a whole lot of sense combined.
option(BUILD_MULA_PLASMA_ACTIVE "Build Mula for Plasma Active" OFF)
option(BUILD_MULA_DESKTOP "Build Mula for Desktop" ON)
option(BUILD_MULA_HARMATTAN "Build Mula for Harmattan" ON)
option(BUILD_MULA_PLASMA "Build Mula for Plasma Desktop" OFF)
option(BUILD_MULA_PLUGINS "Build Mula Plugins" ON)

if(CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    option(INSTALL_CMAKE_FILES "Install the *.cmake files into the CMake root" ON)
else()
    option(INSTALL_CMAKE_FILES "Install the *.cmake files into the CMake root" OFF)
endif()

set(MULA_BUILD_ALL TRUE
    CACHE INTERNAL "Build all of MULA"
)

SET(CMAKE_CXX_FLAGS "-Wall")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -O2 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2")
SET(CMAKE_CXX_FLAGS_DEBUG  "-Wall -g3 -ggdb -O0" CACHE STRING "Debug options." FORCE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

find_package(KDE4)
if(NOT KDE4_FOUND)
	set(BUILD_MULA_PLASMA OFF)
	set(BUILD_MULA_KDE OFF)
    message(STATUS)
    message(STATUS "MULA failure, KDE could not be found.")
    message(STATUS)
endif(NOT KDE4_FOUND)

####### Core #######
add_subdirectory(core)

####### MULA ########
# if(BUILD_MULA_KDE)
# add_subdirectory(kde)
# endif()

####### Plugins #######
add_subdirectory(plugins)

####### MULA Plasma #######
# if(BUILD_MULA_PLASMA)
    # add_subdirectory(plasma)
# endif()

message(STATUS)
message(STATUS "========== MULA Build Information ==========")
message(STATUS "Build Version: ${MULA_VERSION_STRING} (${MULA_VERSION_NAME})")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Install the *.cmake files into CMake root (INSTALL_CMAKE_FILES): ${INSTALL_CMAKE_FILES}")
message(STATUS "Build MULA kde folder (BUILD_MULA_KDE): ${BUILD_MULA_KDE}")
message(STATUS "Build MULA kstardict folder (MULA plugins folder (BUILD_MULA_PLUGINS): ${BUILD_MULA_PLUGINS}")
message(STATUS "Build MULA kstardict folder (MULA plasma folder (BUILD_MULA_PLASMA): ${BUILD_MULA_PLASMA}")
message(STATUS)
message(STATUS "To change any of these options, override them using -D{OPTION_NAME} on the commandline.")
message(STATUS "To build and install MULA, run \"make\" and \"make install\"")
message(STATUS)

if(INSTALL_CMAKE_FILES)
    install(
        FILES
        DESTINATION
        ${CMAKE_ROOT}/Modules
        OPTIONAL
    )
else()
    install(
        FILES
        DESTINATION
        ${SHARE_INSTALL_DIR}/mula/cmake
        OPTIONAL
    )
endif()

###### CPACK part ######
include(CPack)

###### CTEST part ######
include(CTest)

