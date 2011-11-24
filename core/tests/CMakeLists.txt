cmake_minimum_required(VERSION 2.8)

if(NOT MULA_BUILD_ALL)
    project(MulaCoreTest)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
include(MulaMacros)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MULA_CORE_INCLUDES}
)

########### next target ###############

MULA_UNIT_TESTS(
    "${MULA_CORE_LIBS}"     # libraries arguement
    "mulacore"              # modulename argument

    # Source files without the extension
    dictionaryinfotest
    translationtest
)
