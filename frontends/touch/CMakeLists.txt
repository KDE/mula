cmake_minimum_required(VERSION 2.8)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MULA_CORE_INCLUDES}
)

set(touchfrontend_SRCS
    main.cpp
)

SET(GLUON_TOUCHFRONTEND_RCCS
    mula_touchfrontend.qrc
)

# generate rules for building source files from the resources
QT4_ADD_RESOURCES(MULA_TOUCHFRONTEND_RCC_SRCS
    mula_touchfrontend.qrc
)

qt4_automoc(${touchfrontend_SRCS})

if(APPLE)
    add_executable(mula_touchfrontend MACOSX_BUNDLE ${touchfrontend_SRCS})
    #MACOSX_BUNDLE_INFO_STRING
    #MACOSX_BUNDLE_ICON_FILE
    #MACOSX_BUNDLE_GUI_IDENTIFIER
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_NAME GluonPlayer)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${MULA_VERSION_STRING})
    #MACOSX_BUNDLE_COPYRIGHT

    add_custom_command(TARGET gluon_qmlplayer
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaTouchFrontend.app/Contents/Frameworks/
        COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/core/$(CONFIGURATION)/MulaCore.framework ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaTouchFrontend.app/Contents/Frameworks/
    )

set_target_properties(mula_touchfrontend PROPERTIES OUTPUT_NAME MulaTouchfrontend)

else()
    add_executable(mula_touchfrontend ${touchfrontend_SRCS} ${MULA_TOUCHFRONTEND_RCC_SRCS})
endif()
target_link_libraries(mula_touchfrontend ${MULA_CORE_LIBS})

set(MULA_DECLARATIVE_LIBS
    MulaTouchFrontend
    ${MULA_CORE_LIBS}
    ${QT_QTDECLARATIVE_LIBRARY}
    CACHE PATHS "Libraries required for Mula Touch Frontend"
)

if(NOT APPLE)
    install(TARGETS mula_touchfrontend
            DESTINATION ${BIN_INSTALL_DIR}
            COMPONENT mulatouchfrontend)

    install(FILES mula_touchfrontend.desktop
            DESTINATION ${SHARE_INSTALL_DIR}/applications
            COMPONENT mulatouchfrontend)

    if(NOT PLAYER_ICONS_INSTALLED)
        set(PLAYER_ICONS_INSTALLED TRUE)
    endif()
endif()

