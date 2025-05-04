cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ChessEngine VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include SDL paths
set(SDL2_INCLUDE_DIR "C:/Users/matte/C++_libraries/SDL2-devel-2.30.11-VC/SDL2-2.30.11/include")
set(SDL2_LIB_DIR "C:/Users/matte/C++_libraries/SDL2-devel-2.30.11-VC/SDL2-2.30.11/lib/x86")
set(SDL2_TTF_INCLUDE_DIR "C:/Users/matte/C++_libraries/SDL2_ttf-devel-2.24.0-VC/SDL2_ttf-2.24.0/include")
set(SDL2_TTF_LIB_DIR "C:/Users/matte/C++_libraries/SDL2_ttf-devel-2.24.0-VC/SDL2_ttf-2.24.0/lib/x86")


# Add executable
add_executable(ChessEngine main.cpp)

# Add the include directory
include_directories(${CMAKE_SOURCE_DIR}/lib/include)
# Add the library
add_subdirectory(lib)

# Link SDL2 libraries
target_include_directories(ChessEngine PRIVATE ${SDL2_INCLUDE_DIR} PRIVATE ${SDL2_TTF_INCLUDE_DIR})
target_link_directories(ChessEngine PRIVATE ${SDL2_LIB_DIR} PRIVATE ${SDL2_TTF_LIB_DIR})
target_link_libraries(ChessEngine Baccala SDL2 SDL2main SDL2_ttf)

# Copy SDL2.dll to the build folder
add_custom_command(TARGET ChessEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_LIB_DIR}/SDL2.dll" "${SDL2_TTF_LIB_DIR}/SDL2_ttf.dll"
    $<TARGET_FILE_DIR:ChessEngine>)
