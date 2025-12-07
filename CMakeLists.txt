cmake_minimum_required(VERSION 3.10)
project(RoveSoPnP_GUI LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools HINTS "~/Qt/6.9.3/gcc_64")
#find_package(CppLinuxSerial CONFIG REQUIRED HINTS "~/Documents/Misc/CppLinuxSerial")

qt_standard_project_setup()

qt_add_executable(RoveSoPnP_GUI
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

qt_add_translations(
    TARGETS RoveSoPnP_GUI
    TS_FILES RoveSoPnP_GUI_en_US.ts
)

target_link_libraries(RoveSoPnP_GUI
    PRIVATE
        Qt::Core
        Qt::Widgets
        
        #CppLinuxSerial::CppLinuxSerial
)

include(GNUInstallDirs)

target_include_directories(RoveSoPnP_GUI PUBLIC serial/include/)

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

