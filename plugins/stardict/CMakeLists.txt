cmake_minimum_required(VERSION 2.8)

find_package(ZLIB)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}

    ${MULA_CORE_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

set(MULA_STARDICT_PLUGIN_INCLUDES
    ${QT_INCLUDES}
    CACHE PATHS "Includes required for Mula Stardict Plugin"
)

set(stardict_SRCS
    abstractdictionary.cpp
    abstractindexfile.cpp
    dictionary.cpp
    dictionarycache.cpp
    dictionaryzip.cpp
    distance.cpp
    indexfile.cpp
    offsetcachefile.cpp
    #settingsdialog.cpp
    stardict.cpp
    stardictdictionaryinfo.cpp
    stardictdictionarymanager.cpp
    wordentry.cpp
)

set(stardict_HEADERS
    abstractdictionary.h
    abstractindexfile.h
    dictionary.h
    dictionarycache.h
    dictionaryzip.h
    distance.h
    indexfile.h
    offsetcachefile.h
    #settingsdialog.h
    stardict.h
    stardictdictionaryinfo.h
    stardictdictionarymanager.h
    wordentry.h
)

qt4_automoc(${stardict_SRCS})

if(APPLE)
    add_library(mula_plugin_stardict SHARED ${stardict_SRCS} ${stardict_HEADERS})
else()
    add_library(mula_plugin_stardict SHARED ${stardict_SRCS})
endif()

target_link_libraries(mula_plugin_stardict ${MULA_CORE_LIBRARIES} ${ZLIB_LIBRARIES})

if(MULA_BUILD_ALL)
    add_dependencies(mula_plugin_stardict MulaCore)
endif()

set(MULA_STARDICT_PLUGIN_LIBS
    mula_plugin_stardict
    ${MULA_CORE_LIBRARIES}
    CACHE PATHS "StarDict plugin for Mula"
)

set_target_properties(mula_plugin_stardict PROPERTIES VERSION ${MULA_VERSION_STRING} SOVERSION ${MULA_VERSION_STRING} DEFINE_SYMBOL MAKE_MULA_PLUGIN_STARDICT_LIB)

if(APPLE)
    #hack for being able to set headers as public in a osx framework
    list(APPEND stardict_HEADERS
        ${stardicto_HEADERS}
    )

    set_target_properties(mula_plugin_stardict PROPERTIES FRAMEWORK TRUE)
    set_target_properties(mula_plugin_stardict PROPERTIES BUILD_WITH_INSTALL_RPATH 1 INSTALL_NAME_DIR "@executable_path/../Frameworks")
    set_target_properties(mula_plugin_stardict PROPERTIES PUBLIC_HEADER "${stardict_HEADERS}")

    set(MACOSX_FRAMEWORK_IDENTIFIER mula_plguin_stardict)
    set(MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_FRAMEWORK_BUNDLE_VERSION ${MULA_VERSION_STRING})
endif()

install(
    TARGETS     mula_plugin_stardict
    DESTINATION
    ${LIB_INSTALL_DIR}/mula
    RUNTIME     DESTINATION ${BIN_INSTALL_DIR} COMPONENT mulapluginstardict
    LIBRARY     DESTINATION ${LIB_INSTALL_DIR} COMPONENT mulapluginstardict
    ARCHIVE     DESTINATION ${LIB_INSTALL_DIR} COMPONENT mulapluginstardict
    FRAMEWORK   DESTINATION ${LIB_INSTALL_DIR} COMPONENT mulapluginstardict
)

if(BUILD_MULA_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
