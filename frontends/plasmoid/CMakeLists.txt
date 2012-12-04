cmake_minimum_required(VERSION 2.8.9)

add_definitions(
    ${QT_DEFINITIONS}
    ${KDE4_DEFINITIONS}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(mulaplasmaapplet_SRCS
    plasmaapplet.cpp
)

kde4_add_plugin(plasma_applet_mulafrontend ${mulaplasmaapplet_SRCS})

target_link_libraries(plasma_applet_mulafrontend ${MULA_CORE_LIBRARIES})

install(TARGETS plasma_applet_mulafrontend
        DESTINATION ${PLUGIN_INSTALL_DIR}
        COMPONENT mulaplasmoidfrontend)

install(FILES plasma-applet-mulafrontend.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        COMPONENT mulaplasmoidfrontend)
