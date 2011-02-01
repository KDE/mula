cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt4 4.6 REQUIRED)

set(_Init_LIB_SUFFIX "")
if ("${QT_QTCORE_LIBRARY}" MATCHES lib64)
   set(_Init_LIB_SUFFIX 64)
endif ("${QT_QTCORE_LIBRARY}" MATCHES lib64)
if ("${QT_QTCORE_LIBRARY}" MATCHES lib32)
   set(_Init_LIB_SUFFIX 32)
endif ("${QT_QTCORE_LIBRARY}" MATCHES lib32)

set(LIB_SUFFIX		    ${_Init_LIB_SUFFIX} CACHE STRING "The suffix of the system-wide library path")

set(MULA_VERSION_MAJOR     0       CACHE STRING "MULA Major Version")
set(MULA_VERSION_MINOR     1      CACHE STRING "MULA Minor Version")
set(MULA_VERSION_PATCH     0       CACHE STRING "MULA Patch Version")
set(MULA_VERSION_STRING "${MULA_VERSION_MAJOR}.${MULA_VERSION_MINOR}.${MULA_VERSION_PATCH}" CACHE STRING "MULA Version String")
set(MULA_VERSION_NAME      "Electron" CACHE STRING "MULA Version Name")

set(INCLUDE_INSTALL_DIR     ${CMAKE_INSTALL_PREFIX}/include             CACHE PATH "The subdirectory relative to the install prefix where header files will be installed.")
set(LIB_INSTALL_DIR         ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}    CACHE PATH "The subdirectory relative to the install prefix where libraries will be installed.")
set(SHARE_INSTALL_DIR       ${CMAKE_INSTALL_PREFIX}/share               CACHE PATH "The subdiractory relative to the install prefix where data and other files will be installed.")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mula_global.h.in ${CMAKE_CURRENT_BINARY_DIR}/mula_global.h)

include_directories(${QT_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(MULA_CORE_INCLUDES
    ${QT_INCLUDES}
    CACHE PATHS "Includes required for Mula Core"
)

set(MulaCore_SRCS
    debughelper.cpp
)

set(MulaCore_HEADERS
    debughelper.h
    singleton.h
    mula_core_export.h
    dictplugin.h
	${CMAKE_CURRENT_BINARY_DIR}/mula_global.h
)

qt4_automoc(${MulaCore_SRCS})

if(APPLE)
	add_library(MulaCore SHARED ${MulaCore_SRCS} ${MulaCore_HEADERS})
else()
	add_library(MulaCore SHARED ${MulaCore_SRCS})
endif()

target_link_libraries(MulaCore ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTSCRIPT_LIBRARY})

set(MULA_CORE_LIBS
    MulaCore
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
    CACHE PATHS "Libraries required for Mula Core"
)

set_target_properties(MulaCore PROPERTIES VERSION ${MULA_VERSION_STRING} SOVERSION ${MULA_VERSION_STRING} DEFINE_SYMBOL MAKE_MULA_CORE_LIB)

install(TARGETS MulaCore RUNTIME DESTINATION bin LIBRARY DESTINATION ${LIB_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

install(
    FILES
    ${MulaCore_HEADERS}
    DESTINATION ${INCLUDE_INSTALL_DIR}/mula/core
    COMPONENT Devel
)
