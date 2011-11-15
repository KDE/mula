cmake_minimum_required(VERSION 2.8)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MULA_CORE_INCLUDES}
)

set(mula-harmattan_SRCS
    main.cpp
)

SET(MULA-HARMATTAN_RCCS
    mula-harmattan.qrc
)

# generate rules for building source files from the resources
QT4_ADD_RESOURCES(MULA-HARMATTAN_RCC_SRCS
    mula-harmattan.qrc
)

qt4_automoc(${mula-harmattan_SRCS})

if(APPLE)
    add_executable(mula-harmattan MACOSX_BUNDLE ${mula-harmattan_SRCS})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_NAME MulaHarmattan)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${MULA_VERSION_STRING})

    add_custom_command(TARGET mula-harmattan
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaHarmattan.app/Contents/Frameworks/
        COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/core/$(CONFIGURATION)/MulaCore.framework ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaHarmattan.app/Contents/Frameworks/
    )

set_target_properties(mula-harmattan PROPERTIES OUTPUT_NAME MulaHarmattan)

else()
    add_executable(mula-harmattan ${mula-harmattan_SRCS} ${MULA-HARMATTAN_RCC_SRCS})
endif()
target_link_libraries(mula-harmattan ${MULA_CORE_LIBS})

set(MULA_DECLARATIVE_LIBS
    MulaHarmattan
    ${MULA_CORE_LIBS}
    ${QT_QTDECLARATIVE_LIBRARY}
    CACHE PATHS "Libraries required for Mula Harmattan"
)

if(NOT APPLE)
    install(TARGETS mula-harmattan
            DESTINATION ${BIN_INSTALL_DIR}
            COMPONENT mulaharmattan)

    install(FILES mula-harmattan.desktop
            DESTINATION ${SHARE_INSTALL_DIR}/applications
            COMPONENT mulaharmattan)

    if(NOT PLAYER_ICONS_INSTALLED)
        set(PLAYER_ICONS_INSTALLED TRUE)
    endif()
endif()

