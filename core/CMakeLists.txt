cmake_minimum_required(VERSION 2.8.9)

if(NOT MULA_BUILD_ALL)
    project(MulaCore)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Test REQUIRED)

set(_Init_LIB_SUFFIX "")
if ("${Qt5Core_LIBRARIES}" MATCHES lib64)
   set(_Init_LIB_SUFFIX 64)
endif ("${Qt5Core_LIBRARIES}" MATCHES lib64)
if ("${Qt5Core_LIBRARIES}" MATCHES lib32)
   set(_Init_LIB_SUFFIX 32)
endif ("${Qt5Core_LIBRARIES}" MATCHES lib32)

set(LIB_SUFFIX		    ${_Init_LIB_SUFFIX} CACHE STRING "The suffix of the system-wide library path")

set(MULA_VERSION_MAJOR     0       CACHE STRING "MULA Major Version")
set(MULA_VERSION_MINOR     1      CACHE STRING "MULA Minor Version")
set(MULA_VERSION_PATCH     0       CACHE STRING "MULA Patch Version")
set(MULA_VERSION_STRING "${MULA_VERSION_MAJOR}.${MULA_VERSION_MINOR}.${MULA_VERSION_PATCH}" CACHE STRING "MULA Version String")
set(MULA_VERSION_NAME      "Electron" CACHE STRING "MULA Version Name")

set(BIN_INSTALL_DIR         bin                 CACHE INTERNAL  "The subdirectory relative to the install prefix where binary files will be installed.")
set(INCLUDE_INSTALL_DIR     include             CACHE PATH      "The subdirectory relative to the install prefix where header files will be installed.")
set(LIB_INSTALL_DIR         lib${LIB_SUFFIX}    CACHE PATH      "The subdirectory relative to the install prefix where libraries will be installed.")
set(SHARE_INSTALL_DIR       share               CACHE PATH      "The subdiractory relative to the install prefix where data and other files will be installed.")

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( MULA_ARCHITECTURE "64" )
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( MULA_ARCHITECTURE "32" )
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mula_global.h.in ${CMAKE_CURRENT_BINARY_DIR}/mula_global.h)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(MULA_CORE_INCLUDE_DIRS
    ${Qt5Core_INCLUDE_DIRS}
    CACHE PATHS "Includes required for Mula Core"
)

set(MulaCore_SRCS
    debughelper.cpp
    dictionaryinfo.cpp
    dictionarymanager.cpp
    dictionaryplugin.cpp
    directoryprovider.cpp
    pluginmanager.cpp
    translation.cpp
)

set(MulaCore_HEADERS
    debughelper.h
    dictionaryinfo.h
    dictionarymanager.h
    dictionaryplugin.h
    directoryprovider.h
    mula_core_export.h
    pluginmanager.h
    singleton.h
    translation.h
	${CMAKE_CURRENT_BINARY_DIR}/mula_global.h
)

if(APPLE)
	add_library(MulaCore SHARED ${MulaCore_SRCS} ${MulaCore_HEADERS})
else()
	add_library(MulaCore SHARED ${MulaCore_SRCS})
endif()

qt5_use_modules(MulaCore Core)

set(MULA_CORE_LIBRARIES
    MulaCore
    ${Qt5Core_LIBRARIES}
    CACHE PATHS "Libraries required for Mula Core"
)

set_target_properties(MulaCore PROPERTIES VERSION ${MULA_VERSION_STRING} SOVERSION ${MULA_VERSION_STRING} DEFINE_SYMBOL MAKE_MULA_CORE_LIB)

if(APPLE)
    #hack for being able to set headers as public in a osx framework
    list(APPEND MulaCore_HEADERS
        ${MulaCore_HEADERS}
    )

    set_target_properties(MulaCore PROPERTIES FRAMEWORK TRUE)
    set_target_properties(MulaCore PROPERTIES BUILD_WITH_INSTALL_RPATH 1 INSTALL_NAME_DIR "@executable_path/../Frameworks")
    set_target_properties(MulaCore PROPERTIES PUBLIC_HEADER "${GluonCore_HEADERS}")

    set(MACOSX_FRAMEWORK_IDENTIFIER core)
    set(MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_FRAMEWORK_BUNDLE_VERSION ${MULA_VERSION_STRING})
endif()

install(TARGETS MulaCore
    RUNTIME DESTINATION ${BIN_INSTALL_DIR} COMPONENT mulacore
    LIBRARY DESTINATION ${LIB_INSTALL_DIR} COMPONENT mulacore
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR} COMPONENT mulacore
    FRAMEWORK DESTINATION ${LIB_INSTALL_DIR} COMPONENT mulacore
)

install(
    FILES
    ${MulaCore_HEADERS}
    DESTINATION ${INCLUDE_INSTALL_DIR}/mula/core
    COMPONENT Devel
)

if(BUILD_MULA_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
