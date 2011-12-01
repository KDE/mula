cmake_minimum_required(VERSION 2.8)

set(multitran_SRCS
    multitran.cpp
)

qt4_automoc(${multitran_SRCS})
add_library(mula_plugin_multitran SHARED ${multitran_SRCS})

if(MULA_BUILD_ALL)
    add_dependencies(mula_plugin_multitran MULACore)
endif()

set_target_properties(mula_plugin_multitran PROPERTIES VERSION ${MULA_VERSION_STRING} SOVERSION ${MULA_VERSION_STRING} DEFINE_SYMBOL MAKE_MULA_PLUGIN_MULTITRAN_LIB)
target_link_libraries(mula_plugin_multitran ${MULA_CORE_LIBRARIES})

install(
    TARGETS
    mula_plugin_multitran
    DESTINATION
    ${LIB_INSTALL_DIR}/mula
)

if(BUILD_MULA_TESTS)
    enable_testing()
    #add_subdirectory(tests)
endif()
