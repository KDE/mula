cmake_minimum_required(VERSION 2.8)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(kdeextfrontend_SRCS
    main.cpp
    mainwindow.cpp
)

if(APPLE)
	kde4_add_executable(mula_kdeextfrontend MACOSX_BUNDLE ${kdeextfrontend_SRCS})
	#MACOSX_BUNDLE_INFO_STRING
	#MACOSX_BUNDLE_ICON_FILE
	#MACOSX_BUNDLE_GUI_IDENTIFIER
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_NAME MulaKDEEXTFrontend)
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${MULA_VERSION_STRING})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${MULA_VERSION_STRING})
	#MACOSX_BUNDLE_COPYRIGHT

	add_custom_command(TARGET mula_kdeextfrontend
        COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaKDEEXTFrontend.app/Contents/Frameworks/
		COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp ARGS -exclude .DS_Store -exclude CVS -exclude .svn -exclude 'CMakeLists.txt' -resolve-src-symlinks ${CMAKE_BINARY_DIR}/core/$(CONFIGURATION)/MulaCore.framework ${CMAKE_CURRENT_BINARY_DIR}/$(CONFIGURATION)/MulaKDEExtFrontend.app/Contents/Frameworks/
	)

set_target_properties(mula_kdeextfrontend PROPERTIES OUTPUT_NAME MulaKDEExtFrontend)

else()
	kde4_add_app_icon(kdeextfrontend_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*/apps/mula-kdeextfrontend.png")
	kde4_add_executable(mula_kdeextfrontend ${kdeextfrontend_SRCS})
endif()
target_link_libraries(mula_kdeextfrontend ${MULA_CORE_LIBS})

if(NOT APPLE)
    install(TARGETS mula_kdeextfrontend
            DESTINATION ${BIN_INSTALL_DIR}
            COMPONENT mulakdeextfrontend)

    install(FILES ui/mula_kdeextfrontendui.rc
            DESTINATION ${DATA_INSTALL_DIR}/mula_kdeextfrontend
            COMPONENT mulakdeextfrontend)

    install(FILES mula_kdeextfrontend.desktop
            DESTINATION ${SHARE_INSTALL_DIR}/applications
            COMPONENT mulakdeextfrontend)
endif()
