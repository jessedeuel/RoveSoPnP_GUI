cmake_minimum_required(VERSION 3.24.3)
project(RoveSoPnP_GUI LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

if(DEFINED ENV{QT})
    message(STATUS "QT is set to $ENV{QT}")
else()
    message(STATUS "QT environment variable is not set.")
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools HINTS "$ENV{QT}")
#find_package(CppLinuxSerial CONFIG REQUIRED HINTS "~/Documents/Misc/CppLinuxSerial")

qt_standard_project_setup()

qt_add_executable(RoveSoPnP_GUI
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

add_subdirectory(RoveSoPnP_FlowControl)
add_library(RoveVision STATIC RoveSoPnP_Vision)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_Vision/src
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_Vision/src/interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_Vision/src/util
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_Vision/src/vision
)

target_link_libraries(RoveSoPnP_GUI
    PRIVATE
        Qt::Core
        Qt::Widgets
        RoveSoPnP_FlowControl
        RoveVision
)

include(GNUInstallDirs)

#target_link_libraries(RoveSoPnP_GUI PRIVATE )

install(TARGETS RoveSoPnP_GUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET RoveSoPnP_GUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

