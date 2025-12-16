cmake_minimum_required(VERSION 3.24.3)
project(RoveSoPnP_GUI LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED ENV{QT})
    message(STATUS "QT is set to $ENV{QT}")
else()
    message(STATUS "QT environment variable is not set.")
endif()

## Find Python3
find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)

## Find Threads
find_package(Threads REQUIRED)

## Find Quill
find_package(quill REQUIRED)

## Fine nlohmann JSON.
find_package(nlohmann_json 3.2.0 REQUIRED)

## Find Eigen3.
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Find OpenCV
find_package(OpenCV REQUIRED)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Widgets LinguistTools HINTS "~/Qt/6.9.3/gcc_64") #$ENV{QT}
#find_package(CppLinuxSerial CONFIG REQUIRED HINTS "~/Documents/Misc/CppLinuxSerial")

####################################################################################################################
##                                                Build Executable                                                ##
####################################################################################################################

# 1. Qt setup
qt_standard_project_setup()

# Gather source files
file(GLOB_RECURSE EXE_SRC CONFIGURE_DEPENDS "src/*.cpp")

# 2. Define executable
qt_add_executable(RoveSoPnP_GUI
    WIN32 MACOSX_BUNDLE
    ${EXE_SRC}
)

add_subdirectory(RoveSoPnP_FlowControl)
add_library(RoveVision STATIC RoveSoPnP_Vision)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_Vision/src
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_Vision/src/interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_Vision/src/util
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_Vision/src/vision
        ${CMAKE_CURRENT_SOURCE_DIR}/RoveSoPnP_FlowControl/include
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(RoveSoPnP_GUI
    PRIVATE
        Qt::Core
        Qt::Widgets
        RoveFlowControl
        RoveVision
        ${OpenCV_LIBS}
)

add_compile_definitions(BS_THREAD_POOL_ENABLE_PAUSE=1)
add_compile_definitions(BS_THREAD_POOL_ENABLE_PRIORITY=1)
add_compile_definitions(BS_THREAD_POOL_ENABLE_WAIT_DEADLOCK_CHECK=1)

## 5. Compile Options
if (MSVC)
    target_compile_options(RoveVision PRIVATE /W4 /WX)
else()
    target_compile_options(RoveVision PRIVATE -Wall -Wextra -Wpedantic)
endif()

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

