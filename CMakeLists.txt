cmake_minimum_required(VERSION 3.23)
project(Haptic_c__)

set(CMAKE_CXX_STANDARD 11)

include_directories(H:/OpenHaptics/include H:/OpenHaptics/utilities/include)

link_directories(H:/OpenHaptics/lib/x64/Debug H:/OpenHaptics/utilities/lib/x64/Debug)

add_executable(Haptic_c__ main.cpp server.h server.cpp client.cpp)

target_link_libraries(Haptic_c__
        hd hl glut32 glui32 hdu hlu ws2_32)