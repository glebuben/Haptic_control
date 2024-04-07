cmake_minimum_required(VERSION 3.20.2)
project(Haptic_control)

set(CMAKE_CXX_STANDARD 11)

include_directories(C:/OpenHaptics/Developer/3.5.0/include C:/OpenHaptics/Developer/3.5.0/utilities/include C:/OpenHaptics/Developer/3.5.0/Quickhaptics/header)

link_directories(C:/OpenHaptics/Developer/3.5.0/lib/x64/Debug C:/OpenHaptics/Developer/3.5.0/utilities/lib/x64/Debug C:/OpenHaptics/Developer/3.5.0/Quickhaptics/lib/x64/Debug)

add_executable(Haptic_control main.cpp server.h server.cpp)

target_link_libraries(Haptic_control
        hd hl glut32 glui32 hdu hlu ws2_32)