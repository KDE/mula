cmake_minimum_required(VERSION 2.8.9)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${MULA_CORE_INCLUDE_DIRS}
)

set(mula-desktop_SRCS
    main.cpp
)

if(APPLE)
    add_executable(mula-desktop MACOSX_BUNDLE ${mula-desktop_SRCS})

    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_NAME MulaDesktop)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${MULA_VERSION_STRING})

    add_custom_command(TARGET mula-desktop
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaDesktop.app/Contents/Frameworks/
        COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/core/$(CONFIGURATION)/MulaCore.framework ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaDesktop.app/Contents/Frameworks/
    )
set_target_properties(mula-desktop PROPERTIES OUTPUT_NAME MulaDesktop)
else()
    add_executable(mula-desktop ${mula-desktop_SRCS})
endif()
target_link_libraries(mula-desktop ${MULA_CORE_LIBRARIES})

# Use the Widgets module from Qt 5.
qt5_use_modules(mula-desktop Widgets)

if(NOT APPLE)
    install(TARGETS
        mula-desktop

        DESTINATION ${BIN_INSTALL_DIR}
        COMPONENT muladesktop
    )

    install(FILES
        mula-desktop.desktop

        DESTINATION ${SHARE_INSTALL_DIR}/applications
        COMPONENT muladesktop
    )

    # if(NOT PLAYER_ICONS_INSTALLED)
        # add_subdirectory(../icons icons)
        # set(PLAYER_ICONS_INSTALLED TRUE)
    # endif()
endif()
