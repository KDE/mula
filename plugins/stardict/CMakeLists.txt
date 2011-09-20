cmake_minimum_required(VERSION 2.8)

set(stardict_SRCS
    cacheitem.cpp
    dictionary.cpp
    dictionarybase.cpp
    dictionarycache.cpp
    dictionaryzip.cpp
    distance.cpp
    indexfile.cpp
    lib.cpp
    offsetindex.cpp
    stardict.cpp
    stardictdictionaryinfo.cpp
    wordlistindex.cpp
)

set(stardict_HEADERS
    cacheitem.h
    dictionary.h
    dictionarybase.h
    dictionarycache.h
    dictionaryzip.h
    distance.h
    indexfile.h
    lib.h
    offsetindex.h
    stardict.h
    stardictdictionaryinfo.h
    wordlistindex.h
)

qt4_automoc(${stardict_SRCS})

if(APPLE)
    add_library(mula_plugin_stardict SHARED ${stardict_SRCS} ${stardict_HEADERS})
else()
    add_library(mula_plugin_stardict SHARED ${stardict_SRCS})
endif()

if(MULA_BUILD_ALL)
    add_dependencies(mula_plugin_stardict MULACore)
endif()

set_target_properties(mula_plugin_stardict PROPERTIES VERSION ${MULA_VERSION_STRING} SOVERSION ${MULA_VERSION_STRING} DEFINE_SYMBOL MAKE_MULA_PLUGIN_STARDICT_LIB)
target_link_libraries(mula_plugin_stardict ${MULA_CORE_LIBS})

install(
    TARGETS
    mula_plugin_stardict
    DESTINATION
    ${LIB_INSTALL_DIR}/mula
)

if(BUILD_MULA_TESTS)
    enable_testing()
    #add_subdirectory(tests)
endif()
