cmake_minimum_required(VERSION 3.22)
project(glVoxel)

# === Options ===
option(ENABLE_AVX256 "Enable AVX-256 instructions" ON)
option(ENABLE_MULTI_THREADING "Enable multithreaded voxel processing with OpenMP" ON)
option(ENABLE_STL_DEBUG "Enable STL debug mode and DEBUG macro." OFF)
option(ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer (disables AddressSanitizer)" OFF)

# === Compiler Flags ===
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# === Optional AVX ===
if(ENABLE_AVX256)
  add_compile_options(-mavx2)
  message(STATUS "AVX-256 instructions ENABLED.")
else()
  message(STATUS "AVX-256 is DISABLED.")
endif()

# === Source Files ===
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(glVoxel ${SRC_FILES})

# --- Add lib separately to supress debug warnings
file(GLOB_RECURSE LIB_FILES ${CMAKE_SOURCE_DIR}/lib/*.cpp)
add_library(glVoxelLibrary OBJECT ${LIB_FILES})

# --- Suppress all warnings from glVoxelLibrary
target_compile_options(glVoxelLibrary PRIVATE -w)

# --- Add glVoxelLibrary to the binary
target_sources(glVoxel PRIVATE $<TARGET_OBJECTS:glVoxelLibrary>)

# === Static/Shared Object Files ===
file(GLOB_RECURSE SHARED_LIB ${CMAKE_SOURCE_DIR}/bin/*.o)
file(GLOB_RECURSE STATIC_LIB ${CMAKE_SOURCE_DIR}/bin/*.a)

target_sources(glVoxel PRIVATE ${SHARED_LIB})
target_link_libraries(glVoxel PRIVATE ${STATIC_LIB})

# === Multithreading ===
if(ENABLE_MULTI_THREADING)
  find_package(OpenMP REQUIRED)
  if(OpenMP_FOUND)
    target_link_libraries(glVoxel PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(glVoxel PRIVATE ENABLE_MULTI_THREADING)
    message(STATUS "OpenMP multithreading is ENABLED.")
  else()
    message(WARNING "OpenMP requested but not found.")
  endif()
else()
  message(STATUS "OpenMP is DISABLED.")
endif()

# === Libraries ===
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(TBB REQUIRED)
find_library(NOISE_LIB noise PATHS /usr/local/lib REQUIRED)
find_library(NOISEUTILS_LIB noiseutils PATHS /usr/local/lib REQUIRED)

target_link_libraries(glVoxel PRIVATE
  OpenGL
  GLEW
  glfw
  assimp
  TBB::tbb
  ${NOISE_LIB}
  ${NOISEUTILS_LIB}
)

# === Include Directories ===
target_include_directories(glVoxel PRIVATE 
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include/imgui
)
target_include_directories(glVoxelLibrary PRIVATE
  ${CMAKE_SOURCE_DIR}/include/imgui
)

# === Optional: ccache (speed up rebuilds) ===
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
  message(STATUS "ccache is ENABLED.")
endif()

# === Debug Configuration ===
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG -D_GLIBCXX_DEBUG)

  if(ENABLE_STL_DEBUG)
    target_compile_options(glVoxel PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wshadow
      -Wconversion
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
      -Wunused
    )

    message(STATUS "STL debug mode and DEBUG macro is ENABLED.")

    if(ENABLE_THREAD_SANITIZER)
      set(SANITIZE_FLAGS "-fsanitize=thread")
      message(STATUS "Using ThreadSanitizer (no ASan/UBSan).")
    else()
      set(SANITIZE_FLAGS "-fsanitize=undefined,address -fno-omit-frame-pointer")
      message(STATUS "Using AddressSanitizer + UndefinedBehaviorSanitizer.")
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 ${SANITIZE_FLAGS}")
    set(CMAKE_LINKER_FLAGS_DEBUG "${SANITIZE_FLAGS}")
  else()
    message(STATUS "STL debug mode and DEBUG macro is DISABLED.")
  endif()
endif()

# === Release Configuration ===
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto=auto -funroll-loops -fomit-frame-pointer -g0 -s -ffast-math")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto=auto")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-flto=auto")