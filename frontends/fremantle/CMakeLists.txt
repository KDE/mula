cmake_minimum_required(VERSION 2.8)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MULA_CORE_INCLUDES}
)

set(mula-fremantle_SRCS
    main.cpp
)

SET(MULA-FREMANTLE_RCCS
    mula-fremantle.qrc
)

# generate rules for building source files from the resources
QT4_ADD_RESOURCES(MULA-FREMANTLE_RCC_SRCS
    mula-fremantke.qrc
)

qt4_automoc(${mula-fremantle_SRCS})

if(APPLE)
    add_executable(mula-fremantle MACOSX_BUNDLE ${mula-fremantle_SRCS})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_NAME MulaFremantle)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${MULA_VERSION_STRING})

    add_custom_command(TARGET mula-fremantle
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaFremantle.app/Contents/Frameworks/
        COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/core/$(CONFIGURATION)/MulaCore.framework ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaFremantle.app/Contents/Frameworks/
    )

set_target_properties(mula-fremantle PROPERTIES OUTPUT_NAME MulaFremantle)

else()
    add_executable(mula-fremantle ${mula-fremantle_SRCS} ${MULA-FREMANTLE_RCC_SRCS})
endif()
target_link_libraries(mula-fremantle ${MULA_CORE_LIBS})

set(MULA_DECLARATIVE_LIBS
    MulaFremantle
    ${MULA_CORE_LIBS}
    ${QT_QTDECLARATIVE_LIBRARY}
    CACHE PATHS "Libraries required for Mula Fremantle"
)

if(NOT APPLE)
    install(TARGETS mula-fremantle
            DESTINATION ${BIN_INSTALL_DIR}
            COMPONENT mula-fremantle)

    install(FILES mula-fremantle.desktop
            DESTINATION ${SHARE_INSTALL_DIR}/applications
            COMPONENT mulafremantle)

    if(NOT PLAYER_ICONS_INSTALLED)
        set(PLAYER_ICONS_INSTALLED TRUE)
    endif()
endif()

