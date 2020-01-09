cmake_minimum_required(VERSION 2.8)

project(wav2mp3)

find_package (Threads REQUIRED)

set(HEADER_FILES ${YOUR_DIRECTORY}/wave.h ${YOUR_DIRECTORY}/lame_interface.h)

set(SOURCES
   ${CMAKE_CURRENT_SOURCE_DIR}/wave.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/lame_interface.cpp
)
include_directories("/usr/local/include/lame/")
#add_subdirectory() with cmakelists
add_executable(${PROJECT_NAME} "main.cpp" ${SOURCES})
#target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME}  ${CMAKE_SOURCE_DIR}/libmp3lame.a pthread)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
