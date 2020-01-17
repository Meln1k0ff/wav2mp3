cmake_minimum_required(VERSION 2.8)

project(wav2mp3)

find_package (Threads REQUIRED)
set (CMAKE_CXX_STANDARD 11)
set(HEADER_FILES ${YOUR_DIRECTORY}/wave.h ${YOUR_DIRECTORY}/lame_interface.h)

set(SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/wave.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lame_interface.cpp
)

#uncomment this to build under linux.
#include_directories("/usr/local/include/lame/")

#for windows -  %HOMEDRIVE%%HOMEPATH%\Downloads\lame-3.100\lame-3.100\include - put LAME here.
#I am not deeply aware of how to use MinGW, sorry
include_directories("%HOMEDRIVE%%HOMEPATH%/Downloads/lame-3.100/lame-3.100/include")
#
add_executable(${PROJECT_NAME} "main.cpp" ${SOURCES})
#target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
#put lame.a library into the same folder where CMakeLists.txt is

#to build under linux
#target_link_libraries(${PROJECT_NAME}  ${CMAKE_SOURCE_DIR}/libmp3lame.a pthread)
#to build under windows
target_link_libraries(${PROJECT_NAME}  ${CMAKE_SOURCE_DIR}/libmp3lame.a Threads::Threads)
