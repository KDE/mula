cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

find_package(MulaCore REQUIRED)

find_package(Qt4)
if(BUILD_MULA_FREMANTLE)
    if(QT_QTDECLARATIVE_FOUND)
        add_subdirectory(fremantle)
    else(QT_QTDECLARATIVE_FOUND)
        message(WARNING "Qt installation lacks Qt Declarative - disabling the Fremantle Frontend")
    endif(QT_QTDECLARATIVE_FOUND)
endif()

if(BUILD_MULA_HARMATTAN)
    if(QT_QTDECLARATIVE_FOUND)
        add_subdirectory(harmattan)
    else(QT_QTDECLARATIVE_FOUND)
        message(WARNING "Qt installation lacks Qt Declarative - disabling the Harmattan Frontend")
    endif(QT_QTDECLARATIVE_FOUND)
endif()

if(BUILD_MULA_DESKTOP)
    if(QT_QTDECLARATIVE_FOUND)
        add_subdirectory(harmattan)
    else(QT_QTDECLARATIVE_FOUND)
        message(WARNING "Qt installation lacks Qt Declarative - disabling the Desktop Frontend")
    endif(QT_QTDECLARATIVE_FOUND)
endif()

if(BUILD_MULA_PLASMOID)
    find_package(KDE4)
    if(KDE4_FOUND)
        include_directories(${KDE4_INCLUDES})
        add_subdirectory(plasmoid)
    else(KDE4_FOUND)
        message(STATUS "KDE libraries are not found - disabling the Plasmoid Desktop Frontend")
    endif(KDE4_FOUND)
endif()

if(BUILD_MULA_PLASMA_ACTIVE)
    find_package(KDE4)
    if(KDE4_FOUND)
        include_directories(${KDE4_INCLUDES})
        add_subdirectory(plasma-active)
    else(KDE4_FOUND)
        message(STATUS "KDE libraries are not found - disabling the Plasma Active Frontend")
    endif(KDE4_FOUND)
endif()
