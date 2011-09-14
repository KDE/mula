cmake_minimum_required(VERSION 2.8)

set(swac_SRCS
    swac.cpp
)

qt4_automoc(${swac_SRCS})
add_library(mula_plugin_swac SHARED ${swac_SRCS})

if(MULA_BUILD_ALL)
    add_dependencies(mula_plugin_swac MULACore)
endif()

set_target_properties(mula_plugin_swac PROPERTIES VERSION ${MULA_VERSION_STRING} SOVERSION ${MULA_VERSION_STRING} DEFINE_SYMBOL MAKE_MULA_PLUGIN_SWAC_LIB)
target_link_libraries(mula_plugin_swac ${MULA_CORE_LIBS})

install(
    TARGETS
    mula_plugin_swac
    DESTINATION
    ${LIB_INSTALL_DIR}/mula
)

if(BUILD_MULA_TESTS)
    enable_testing()
    #add_subdirectory(tests)
endif()
