cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ChessEngine2 VERSION 1.0)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Executable
add_executable(ChessEngine2 main.cpp)

# Include directory for your custom lib
include_directories(${CMAKE_SOURCE_DIR}/lib/include)

# Add your custom lib (assuming CMakeLists.txt is inside lib/)
add_subdirectory(lib)

# Link your engine against the custom library
target_link_libraries(ChessEngine2 PRIVATE Baccala)

# ---------- Compiler options ----------
if(MSVC)
    # Debug settings (safe, verbose warnings)
    target_compile_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Debug>:/W4 /RTC1 /Od /Zi>
    )

    # Release settings (maximum optimization + AVX2)
    target_compile_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Release>:/O2 /GL /arch:AVX2>
    )
    target_link_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
else()
    # For GCC/Clang (if you ever build outside MSVC)
    target_compile_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Debug>:-Wall -Wextra -g -O0 -fsanitize=address>
        $<$<CONFIG:Release>:-O3 -march=native -flto>
    )
    target_link_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>
        $<$<CONFIG:Release>:-flto>
    )
endif()