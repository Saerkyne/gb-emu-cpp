cmake_minimum_required(VERSION 3.25)
project(gb-emu-cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- SDL3 -------------------------------------------------------------
include(FetchContent)

set(SDL_SHARED       ON  CACHE BOOL "" FORCE)
set(SDL_STATIC       OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL      OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-3.4.16      # <-- pin a real tag from the releases page
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(SDL3)

# --- your target ------------------------------------------------------
add_executable(gb-emu-cpp
  src/main.cpp
  src/cart.cpp
  src/cart.h
  src/cart_type.h
  src/cpu.cpp
  src/cpu.h
  src/cpu_instructions.cpp
  src/cpu_instructions.h
  src/cpu_routines.h
  src/debug_log.cpp
  src/debug_log.h
  src/emulator_core.cpp
  src/emulator_core.h
  src/interrupts.cpp
  src/interrupts.h
  src/memory_bus.cpp
  src/memory_bus.h
  src/resource.h
  src/timer.cpp
  src/timer.h
)

target_link_libraries(gb-emu-cpp PRIVATE SDL3::SDL3)

target_compile_options(gb-emu-cpp PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive- /W4 /utf-8>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Copy SDL3.dll next to the exe on Windows
add_custom_command(TARGET gb-emu-cpp POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_RUNTIME_DLLS:gb-emu-cpp> $<TARGET_FILE_DIR:gb-emu-cpp>
  COMMAND_EXPAND_LISTS
)