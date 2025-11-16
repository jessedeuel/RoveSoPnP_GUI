cmake_minimum_required(VERSION 3.19)
project(RoveSoPnP_GUI LANGUAGES CXX)

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools HINTS "~/Qt/6.9.3/gcc_64")

qt_standard_project_setup()

qt_add_executable(RoveSoPnP_GUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

qt_add_translations(
    TARGETS RoveSoPnP_GUI
    TS_FILES RoveSoPnP_GUI_en_US.ts
)

target_link_libraries(RoveSoPnP_GUI
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

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
