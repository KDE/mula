cmake_minimum_required(VERSION 2.8)

if(NOT MULA_BUILD_ALL)
    project(MulaStarDictPluginTest)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
include(MulaMacros)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MULA_PLUGIN_STARDICT_INCLUDES}
)

########### next target ###############

MULA_UNIT_TESTS(
    "${MULA_STARDICT_PLUGIN_LIBS}"      # libraries arguement
    "stardictplugin"                    # modulename argument

    # Source files without the extension
)
