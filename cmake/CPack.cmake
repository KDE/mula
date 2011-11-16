include(InstallRequiredSystemLibraries)

# Packaging options
# Component based settings
set(CPACK_COMPONENTS_ALL
    mulacore
    mulapluginmultitran
    mulapluginstardict
    mulapluginswac
    mulapluginweb
)

# Core library
set(CPACK_COMPONENT_MULACORE_DISPLAY_NAME "Mula Core Library")
set(CPACK_COMPONENT_MULACORE_DESCRIPTION
    "MulaCore is a thin library which provides the core functionality for standalone
    and plugin development purposes"
)
set(CPACK_COMPONENT_MULACORE_GROUP "MULALIBS")
set(CPACK_COMPONENT_MULACORE_INSTALL_TYPES Minimal Full)

# Multitran plugin
set(CPACK_COMPONENT_MULAPLUGINMULTITRAN_DISPLAY_NAME "Mula Multitran plugin")
set(CPACK_COMPONENT_MULAPLUGINMULTITRAN_DESCRIPTION
    "Open platform for professional translators plugin"
)
set(CPACK_COMPONENT_MULAPLUGINMULTITRAN_DEPENDS mulacore)
set(CPACK_COMPONENT_MULAPLUGINMULTITRAN_GROUP "MULAPLUGINS")
set(CPACK_COMPONENT_MULAPLUGINMULTITRAN_INSTALL_TYPES Full)

# StarDict plugin
set(CPACK_COMPONENT_MULAPLUGINSTARDICT_DISPLAY_NAME "Mula StarDict plugin")
set(CPACK_COMPONENT_MULAPLUGINSTARDICT_DESCRIPTION
    "StarDict dicitonary format support plugin"
)
set(CPACK_COMPONENT_MULAPLUGINSTARDICT_DEPENDS mulacore)
set(CPACK_COMPONENT_MULAPLUGINSTARDICT_GROUP "MULAPLUGINS")
set(CPACK_COMPONENT_MULAPLUGINSTARDICT_INSTALL_TYPES Full)

# Swac plugin
set(CPACK_COMPONENT_MULAPLUGINSWAC_DISPLAY_NAME "Mula Swac plugin")
set(CPACK_COMPONENT_MULAPLUGINSWAC_DESCRIPTION
    "This plugin allows you to easily memorize word sounds from Swac
    collections."
)
set(CPACK_COMPONENT_MULAPLUGINSWAC_DEPENDS mulacore)
set(CPACK_COMPONENT_MULAPLUGINSWAC_GROUP "MULAPLUGINS")
set(CPACK_COMPONENT_MULAPLUGINSWAC_INSTALL_TYPES Full)

# Web plugin
set(CPACK_COMPONENT_MULAPLUGINWEB_DISPLAY_NAME "Mula Web plugin")
set(CPACK_COMPONENT_MULAPLUGINWEB_DESCRIPTION
    "Support for WEB dictionaries"
)
set(CPACK_COMPONENT_MULAPLUGINWEB_DEPENDS mulacore)
set(CPACK_COMPONENT_MULAPLUGINWEB_GROUP "MULAPLUGINS")
set(CPACK_COMPONENT_MULAPLUGINWEB_INSTALL_TYPES Full)

# Plugins
set(CPACK_COMPONENT_GROUP_MULAPLUGINS_DISPLAY_NAME "Mula plugins")
set(CPACK_COMPONENT_GROUP_MULAPLUGINS_DESCRIPTION
    "They provide extended functionality to assist in looking up words, and they
    provide functionality for users to listen to the pronunciation as well."
)

# Desktop frontend
set(CPACK_COMPONENT_MULADESKTOP_DISPLAY_NAME "Mula Desktop Frontend application")
set(CPACK_COMPONENT_MULADESKTOP_DESCRIPTION
    "Mula Desktop Frontend Application mostly meant for easy to Desktop usage"
)
set(CPACK_COMPONENT_MULADESKTOP_DEPENDS mulacore)
set(CPACK_COMPONENT_MULADESKTOP_GROUP "MULAFRONTENDS")
set(CPACK_COMPONENT_MULADESKTOP_INSTALL_TYPES Full)

# Fremantle Frontend
set(CPACK_COMPONENT_MULAFREMANTLE_DISPLAY_NAME "Mula Fremantle Frontend Application")
set(CPACK_COMPONENT_MULAFREMANTLE_DESCRIPTION
    "Mula Fremantle Frontend Application mostly meant for Maemo5, Fremantle handsets,
    by using the relevant Qt Quick technology"
)
set(CPACK_COMPONENT_MULAFREMANTLE_DEPENDS mulacore)
set(CPACK_COMPONENT_MULAFREMANTLE_GROUP "MULAFRONTENDS")
set(CPACK_COMPONENT_MULAFREMANTLE_INSTALL_TYPES Full)

# Harmattan Frontend
set(CPACK_COMPONENT_MULAHARMATTAN_DISPLAY_NAME "Mula Harmattan Frontend Application")
set(CPACK_COMPONENT_MULAHARMATTAN_DESCRIPTION
    "Mula Harmattan Frontend Application mostly meant for Maemo6, Harmattan handsets,
    by using the relevant Qt Quick technology"
)
set(CPACK_COMPONENT_MULAHARMATTAN_DEPENDS mulacore)
set(CPACK_COMPONENT_MULAHARMATTAN_GROUP "MULAFRONTENDS")
set(CPACK_COMPONENT_MULAHARMATTAN_INSTALL_TYPES Full)

# Plasma Active Frontend
set(CPACK_COMPONENT_MULAPLASMAACTIVE_DISPLAY_NAME "Mula Plasma Active Frontend Application")
set(CPACK_COMPONENT_MULAPLASMAACTIVE_DESCRIPTION
    "Mula Plasma Active Frontend Application is a full-fledged and powerful
    application for Plasma Active devices"
)
set(CPACK_COMPONENT_MULAPLASMAACTIVE_DEPENDS mulacore)
set(CPACK_COMPONENT_MULAPLASMAACTIVE_GROUP "MULAFRONTENDS")
set(CPACK_COMPONENT_MULAPLASMAACTIVE_INSTALL_TYPES Full)

# Plasmoid Frontend
set(CPACK_COMPONENT_MULAPLASMOID_DISPLAY_NAME "Mula Plasmoid Frontend Application")
set(CPACK_COMPONENT_MULAPLASMOID_DESCRIPTION
    "Mula Plasmoid Frontend Application is a full-fledged and powerful Plasmoid
    applet on Desktop"
)
set(CPACK_COMPONENT_MULAPLASMOID_DEPENDS mulacore)
set(CPACK_COMPONENT_MULAPLASMOID_GROUP "MULAFRONTENDS")
set(CPACK_COMPONENT_MULAPLASMOID_INSTALL_TYPES Full)

# Frontends
set(CPACK_COMPONENT_GROUP_MULAFRONTENDS_DISPLAY_NAME "Mula Frontend Applications")
set(CPACK_COMPONENT_GROUP_MULAFRONTENDS_DISPLAY_DESCRIPTION "Mula Frontend Applications")

set(CPACK_ALL_INSTALL_TYPES Minimal Full)

# Common package generation settings
set(CPACK_PACKAGE_NAME "Mula")
set(CPACK_PACKAGE_VENDOR "Mula")
set(CPACK_PACKAGE_VERSION "${MULA_VERSION_MAJOR}.${MULA_VERSION_MINOR}.${MULA_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A free and open source, pluginable offline and online dictionary with text-to-speech feature")
set(CPACK_PACKAGE_FILE_NAME "Mula-${MULA_VERSION_STRING}")
set(CPACK_GENERATOR "RPM;DEB;TGZ;NSIS")
set(CPACK_PACKAGE_CONTACT "Laszlo Papp <lpapp@kde.org>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Mula-${MULA_VERSION_STRING}")
set(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git/;
    /\\\\.gitignore$;
    /\\\\.reviewboardrc$;
    /\\\\astylerc$;
    /#;
    \\\\.krazy$;
    \\\\.swp$;
    \\\\.#;
    .*~;
    cscope.*"
)

# Debian package generation
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-gui (>= 4.7)")
set(CPACK_DEBIAN_PACKAGE_SECTION "Utils")

# Rpm package generation
#set(CPACK_RPM_PACKAGE_REQUIRES "libqt-devel (>= 4.7.1)")

# Windows NSIS
#set(CPACK_NSIS_MUI_ICON "installer.ico")
#set(CPACK_NSIS_MUI_UNIICON "uninstaller.ico")
#set(CPACK_PACKAGE_ICON "installer.bmp")
#set(CPACK_NSIS_COMPRESSOR "/SOLID lzma")
#set(CPACK_NSIS_INSTALLED_ICON_NAME "")
set(CPACK_NSIS_HELP_LINK "")
set(CPACK_NSIS_URL_INFO_ABOUT "")
set(CPACK_NSIS_CONTACT "lpapp@kde.org")

