cmake_minimum_required(VERSION 3.10)
project(Baccala)

add_library(Baccala STATIC src/Baccala.cpp src/Position.cpp src/Utilities.cpp src/Bitboards.cpp src/TranspositionTable.cpp src/Game.cpp)

target_include_directories(Baccala 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include 
)