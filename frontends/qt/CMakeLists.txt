cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(qtfrontend_SRCS
    main.cpp
    mainwindow.cpp
)

qt4_automoc(${qtfrontend_SRCS})

if(APPLE)
    add_executable(mula_qtfrontend MACOSX_BUNDLE ${qtfrontend_SRCS})

    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_NAME MulaQtFrontend)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${MULA_VERSION_STRING})

    add_custom_command(TARGET mula_qtfrontend
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaQtPlayer.app/Contents/Frameworks/
        COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/core/$(CONFIGURATION)/MulaCore.framework ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaQtFrontend.app/Contents/Frameworks/
    )
set_target_properties(mula_qtfrontend PROPERTIES OUTPUT_NAME MulaQtFrontend)
else()
    add_executable(mula_qtfrontend ${qtfrontend_SRCS})
endif()
target_link_libraries(mula_qtfrontend ${MULA_CORE_LIBS})

if(NOT APPLE)
    install(TARGETS
        mula_qtfrontend

        DESTINATION ${BIN_INSTALL_DIR}
        COMPONENT mulaqtfrontend
    )

    install(FILES
        mula_qtfrontend.desktop

        DESTINATION ${SHARE_INSTALL_DIR}/applications
        COMPONENT mulaqtfrontend
    )

    if(NOT PLAYER_ICONS_INSTALLED)
        add_subdirectory(../icons icons)
        set(PLAYER_ICONS_INSTALLED TRUE)
    endif()
endif()
