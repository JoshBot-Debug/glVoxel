cmake_minimum_required(VERSION 3.22)
project(glVoxel)

option(ENABLE_AVX256 "Enable AVX-256 instructions" OFF)

if(ENABLE_AVX256)
  add_compile_options(-mavx2)
  message(STATUS "AVX-256 instructions enabled.")
else()
  message(WARNING "AVX-256 is not enabled. Falling back to default instructions.")
endif()

# Find packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_library(NOISE_LIB noise PATHS /usr/local/lib REQUIRED)
find_library(NOISEUTILS_LIB noiseutils PATHS /usr/local/lib REQUIRED)
find_package(TBB REQUIRED)
find_package(OpenMP REQUIRED)

# Collect all source files (.cpp files)
file(GLOB_RECURSE SOURCES_FILES
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/lib/*.cpp
)

# Include all source files (.cpp files)
add_executable(glVoxel ${SOURCES_FILES})


# Specify include directories. (header files)
target_include_directories(glVoxel PRIVATE 
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include/imgui
)

# Avoid shared libs, use static linking instead
# Collect all bin files (.o files)
file(GLOB_RECURSE SHARED_LIB ${CMAKE_SOURCE_DIR}/bin/*.o)
target_sources(glVoxel PRIVATE ${SHARED_LIB})

# Collect all bin files (.a files)
file(GLOB_RECURSE STATIC_LIB ${CMAKE_SOURCE_DIR}/bin/*.a)

# Link libraries
target_link_libraries(glVoxel PRIVATE
  OpenGL
  GLEW
  glfw
  assimp
  TBB::tbb
  OpenMP::OpenMP_CXX
  ${NOISE_LIB}
  ${NOISEUTILS_LIB}
  ${STATIC_LIB}
)

# Specify C++ standard
set_target_properties(glVoxel PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# Enable the best optimization settings
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -funroll-loops -fomit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-flto")

# Disable debug flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0")

# Strip symbols for production builds
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

# Enable fast math
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()