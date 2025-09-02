cmake_minimum_required(VERSION 3.10)
project(Baccala)

set(SDL2_INCLUDE_DIR "C:/Users/matte/C++_libraries/SDL2-devel-2.30.11-VC/SDL2-2.30.11/include")
set(SDL2_LIB_DIR "C:/Users/matte/C++_libraries/SDL2-devel-2.30.11-VC/SDL2-2.30.11/lib/x64")
set(SDL2_TTF_INCLUDE_DIR "C:/Users/matte/C++_libraries/SDL2_ttf-devel-2.24.0-VC/SDL2_ttf-2.24.0/include")
set(SDL2_TTF_LIB_DIR "C:/Users/matte/C++_libraries/SDL2_ttf-devel-2.24.0-VC/SDL2_ttf-2.24.0/lib/x64")
set(SDL2_IMAGE_INCLUDE_DIR "C:/Users/matte/C++_libraries/SDL2_image-devel-2.8.8-VC/SDL2_image-2.8.8/include")
set(SDL2_IMAGE_LIB_DIR     "C:/Users/matte/C++_libraries/SDL2_image-devel-2.8.8-VC/SDL2_image-2.8.8/lib/x64")


add_library(Baccala STATIC 
    src/Baccala.cpp 
    src/Position.cpp 
    src/Utilities.cpp 
    src/Bitboards.cpp 
    src/TranspositionTable.cpp 
    src/Game.cpp
)

target_include_directories(Baccala 
    PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	C:/Users/matte/C++_libraries/SDL2-devel-2.30.11-VC/SDL2-2.30.11/include
        C:/Users/matte/C++_libraries/SDL2_ttf-devel-2.24.0-VC/SDL2_ttf-2.24.0/include
	C:/Users/matte/C++_libraries/SDL2_image-devel-2.8.8-VC/SDL2_image-2.8.8/include	
)

# Includi SDL2 (ereditando da target padre)
target_link_libraries(Baccala PUBLIC SDL2 SDL2main SDL2_ttf SDL2_image)