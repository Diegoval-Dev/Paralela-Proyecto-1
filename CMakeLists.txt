cmake_minimum_required(VERSION 3.22) # tu CMake es 4.0-rc3, 3.22+ va bien
project(ScreensaverOmp CXX)

# ---- Config global ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Flags razonables
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-O3 -march=native -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-O3 -march=native -Wall -Wextra -Wpedantic)
elseif (MSVC)
  add_compile_options(/O2 /permissive- /W4)
endif()

# ---- Opciones del proyecto ----
option(ENABLE_OPENMP "Enable OpenMP parallel versions" ON)
option(ENABLE_SDL2 "Build SDL2 renderer" OFF)
option(ENABLE_SFML "Build SFML renderer" OFF)

# ---- OpenMP ----
if (ENABLE_OPENMP)
  find_package(OpenMP)
  if (OpenMP_CXX_FOUND)
    message(STATUS "OpenMP enabled")
  else()
    message(WARNING "OpenMP not found. Parallel targets will be skipped.")
    set(ENABLE_OPENMP OFF)
  endif()
endif()

# ---- Targets comunes (lib de dominio) ----
add_library(core STATIC
  src/core/physics.cpp
  src/core/grid.cpp
  src/omp/update_seq.cpp
  src/omp/update_omp_for.cpp
  src/omp/update_omp_simd.cpp
  src/omp/update_omp_tasks.cpp
)
target_include_directories(core PUBLIC src)
if (ENABLE_OPENMP AND OpenMP_CXX_FOUND)
  target_link_libraries(core PUBLIC OpenMP::OpenMP_CXX)
endif()

# ---- Renderers ----
add_library(renderer_dummy STATIC src/gfx/renderer_dummy.cpp)
target_include_directories(renderer_dummy PUBLIC src)

if (ENABLE_SDL2)
  # Ruta A: con vcpkg: -DCMAKE_TOOLCHAIN_FILE=[vcpkg]/scripts/buildsystems/vcpkg.cmake
  find_package(SDL2 CONFIG QUIET)
  if (SDL2_FOUND)
    add_library(renderer_sdl2 STATIC src/gfx/renderer_sdl2.cpp)
    target_link_libraries(renderer_sdl2 PRIVATE SDL2::SDL2)
    target_include_directories(renderer_sdl2 PUBLIC src)
  else()
    message(WARNING "SDL2 not found. Disabling ENABLE_SDL2.")
    set(ENABLE_SDL2 OFF)
  endif()
endif()

if (ENABLE_SFML)
  # Ruta B: FindSFML.cmake o vcpkg
  find_package(SFML 2.6 COMPONENTS system window graphics QUIET)
  if (SFML_FOUND)
    add_library(renderer_sfml STATIC src/gfx/renderer_sfml.cpp)
    target_link_libraries(renderer_sfml PRIVATE sfml-graphics sfml-window sfml-system)
    target_include_directories(renderer_sfml PUBLIC src)
  else()
    message(WARNING "SFML not found. Disabling ENABLE_SFML.")
    set(ENABLE_SFML OFF)
  endif()
endif()

# ---- Ejecutables (modos) ----
# Modo secuencial
add_executable(seq src/app/main.cpp)
target_compile_definitions(seq PRIVATE BUILD_MODE_SEQ)
target_link_libraries(seq PRIVATE core renderer_dummy)

# Modo OMP for
if (ENABLE_OPENMP)
  add_executable(omp_for src/app/main.cpp)
  target_compile_definitions(omp_for PRIVATE BUILD_MODE_OMP_FOR)
  target_link_libraries(omp_for PRIVATE core renderer_dummy OpenMP::OpenMP_CXX)

  add_executable(omp_simd src/app/main.cpp)
  target_compile_definitions(omp_simd PRIVATE BUILD_MODE_OMP_SIMD)
  target_link_libraries(omp_simd PRIVATE core renderer_dummy OpenMP::OpenMP_CXX)

  add_executable(omp_tasks src/app/main.cpp)
  target_compile_definitions(omp_tasks PRIVATE BUILD_MODE_OMP_TASKS)
  target_link_libraries(omp_tasks PRIVATE core renderer_dummy OpenMP::OpenMP_CXX)
endif()

# Ejecutables con renderer real (si activas SDL2/SFML)
if (ENABLE_SDL2)
  add_executable(seq_sdl2 src/app/main.cpp)
  target_compile_definitions(seq_sdl2 PRIVATE BUILD_MODE_SEQ USE_SDL2)
  target_link_libraries(seq_sdl2 PRIVATE core renderer_sdl2)
  if (ENABLE_OPENMP)
    add_executable(omp_for_sdl2 src/app/main.cpp)
    target_compile_definitions(omp_for_sdl2 PRIVATE BUILD_MODE_OMP_FOR USE_SDL2)
    target_link_libraries(omp_for_sdl2 PRIVATE core renderer_sdl2 OpenMP::OpenMP_CXX)
  endif()
endif()

if (ENABLE_SFML)
  add_executable(seq_sfml src/app/main.cpp)
  target_compile_definitions(seq_sfml PRIVATE BUILD_MODE_SEQ USE_SFML)
  target_link_libraries(seq_sfml PRIVATE core renderer_sfml)
  if (ENABLE_OPENMP)
    add_executable(omp_for_sfml src/app/main.cpp)
    target_compile_definitions(omp_for_sfml PRIVATE BUILD_MODE_OMP_FOR USE_SFML)
    target_link_libraries(omp_for_sfml PRIVATE core renderer_sfml OpenMP::OpenMP_CXX)
  endif()
endif()

# ---- Instalación simple (bin + scripts) ----
install(TARGETS seq
  RUNTIME DESTINATION bin
)
if (ENABLE_OPENMP)
  install(TARGETS omp_for omp_simd omp_tasks RUNTIME DESTINATION bin)
endif()
