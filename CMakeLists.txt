cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ChessEngine2 VERSION 1.0)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ---- Percorsi librerie esterne (modifica questi se cambi cartelle) ----
set(SDL2_INCLUDE_DIR "C:/Users/matte/C++_libraries/SDL2-devel-2.30.11-VC/SDL2-2.30.11/include")
set(SDL2_LIB_DIR     "C:/Users/matte/C++_libraries/SDL2-devel-2.30.11-VC/SDL2-2.30.11/lib/x64")
set(SDL2_TTF_INCLUDE_DIR "C:/Users/matte/C++_libraries/SDL2_ttf-devel-2.24.0-VC/SDL2_ttf-2.24.0/include")
set(SDL2_TTF_LIB_DIR     "C:/Users/matte/C++_libraries/SDL2_ttf-devel-2.24.0-VC/SDL2_ttf-2.24.0/lib/x64")
set(SDL2_IMAGE_INCLUDE_DIR "C:/Users/matte/C++_libraries/SDL2_image-devel-2.8.8-VC/SDL2_image-2.8.8/include")
set(SDL2_IMAGE_LIB_DIR     "C:/Users/matte/C++_libraries/SDL2_image-devel-2.8.8-VC/SDL2_image-2.8.8/lib/x64")


# ---- Executable ----
add_executable(ChessEngine2 main.cpp)

# Include directories
target_include_directories(ChessEngine2 PRIVATE 
    ${SDL2_INCLUDE_DIR} 
    ${SDL2_TTF_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/include
)

# Link directories
target_link_directories(ChessEngine2 PRIVATE 
    ${SDL2_LIB_DIR} 
    ${SDL2_TTF_LIB_DIR}
    ${SDL2_IMAGE_LIB_DIR}
)


# ---- Subdirectory con la tua libreria custom ----
add_subdirectory(lib)
target_link_libraries(ChessEngine2 PRIVATE Baccala PUBLIC SDL2 SDL2main SDL2_ttf SDL2_image)

# ---------- Compiler options ----------
if(MSVC)
    target_compile_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Debug>:/W4 /RTC1 /Od /Zi>
        $<$<CONFIG:Release>:/O2 /GL /arch:AVX2>
    )
    target_link_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
else()
    target_compile_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Debug>:-Wall -Wextra -g -O0 -fsanitize=address>
        $<$<CONFIG:Release>:-O3 -march=native -flto>
    )
    target_link_options(ChessEngine2 PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address>
        $<$<CONFIG:Release>:-flto>
    )
endif()

# Copy DLLs accanto all’eseguibile
add_custom_command(TARGET ChessEngine2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_LIB_DIR}/SDL2.dll"
    "${SDL2_TTF_LIB_DIR}/SDL2_ttf.dll"
    "${SDL2_IMAGE_LIB_DIR}/SDL2_image.dll"
    $<TARGET_FILE_DIR:ChessEngine2>
)