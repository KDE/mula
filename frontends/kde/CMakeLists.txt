cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kdefrontend_SRCS
    main.cpp
    mainwindow.cpp
)

if(APPLE)
	kde4_add_executable(mula_kdefrontend MACOSX_BUNDLE ${kdefrontend_SRCS})
	#MACOSX_BUNDLE_INFO_STRING
	#MACOSX_BUNDLE_ICON_FILE
	#MACOSX_BUNDLE_GUI_IDENTIFIER
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_NAME MulaKDEFrontend)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${MULA_VERSION_STRING})
	#MACOSX_BUNDLE_COPYRIGHT

	add_custom_command(TARGET gluon_kdeplayer
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaKDEPLayer.app/Contents/Frameworks/
		COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/core/$(CONFIGURATION)/MulaCore.framework ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaKDEFrontend.app/Contents/Frameworks/
	)

set_target_properties(mula_kdefrontend PROPERTIES OUTPUT_NAME MulaKDEFrontend)

else()
	kde4_add_app_icon(kdefrontend_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*/apps/mula-kdefrontend.png")
	kde4_add_executable(mula_kdefrontend ${mulafrontend_SRCS})
endif()
target_link_libraries(mula_kdefrontend ${MULA_CORE_LIBS})

if(NOT APPLE)
    install(TARGETS mula_kdefrontend
            DESTINATION ${BIN_INSTALL_DIR}
            COMPONENT mulakdefrontend)

    install(FILES mula_kdefrontend.desktop
            DESTINATION ${SHARE_INSTALL_DIR}/applications
            COMPONENT mulakdefrontend)

endif()
