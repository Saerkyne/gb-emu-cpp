cmake_minimum_required(VERSION 3.25)
project(gb-emu-cpp VERSION 0.5.0 LANGUAGES CXX)

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
  src/dma.cpp
  src/dma.h
  src/emulator_core.cpp
  src/emulator_core.h
  src/gfx.cpp
  src/gfx.h
  src/interrupts.cpp
  src/interrupts.h
  src/joypad.cpp
  src/joypad.h
  src/memory_bus.cpp
  src/memory_bus.h
  src/ppu.cpp
  src/ppu.h
  src/rtc.cpp
  src/rtc.h
  src/timer.cpp
  src/timer.h
)

target_link_libraries(gb-emu-cpp PRIVATE SDL3::SDL3)

target_compile_options(gb-emu-cpp PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive- /W4 /utf-8>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Copy SDL3.dll next to the exe on Windows.
# Linux has no DLLs: the list would be empty, the copy command would fail,
# and make would delete the freshly linked executable. The rpath CMake
# embeds in the Linux binary already points at libSDL3.so.
if(WIN32)
  add_custom_command(TARGET gb-emu-cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:gb-emu-cpp> $<TARGET_FILE_DIR:gb-emu-cpp>
    COMMAND_EXPAND_LISTS
  )
endif()

# Keep runtime relative paths based on repo root during debugging in Visual Studio
set_target_properties(gb-emu-cpp PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Linux desktop entry. It puts the emulator in file managers' "Open With" menus and
# lets you drop ROMs onto its launcher. The generated file points at this build's
# executable, so it stays valid across rebuilds; reinstall it only if the build folder moves.
if(UNIX AND NOT APPLE)
  file(GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gb-emu-cpp.desktop"
    INPUT "${CMAKE_CURRENT_SOURCE_DIR}/platform/linux/gb-emu-cpp.desktop.in"
  )

  # Per-user applications folder from the XDG spec
  if(DEFINED ENV{XDG_DATA_HOME} AND NOT "$ENV{XDG_DATA_HOME}" STREQUAL "")
    set(GB_EMU_APPLICATIONS_DIR "$ENV{XDG_DATA_HOME}/applications")
  else()
    set(GB_EMU_APPLICATIONS_DIR "$ENV{HOME}/.local/share/applications")
  endif()

  # Refresh the menu caches if the tools exist. update-desktop-database covers
  # GNOME-style "Open With" lists; kbuildsycoca is KDE's cache, which Dolphin reads.
  find_program(GB_EMU_UPDATE_DESKTOP_DATABASE update-desktop-database)
  find_program(GB_EMU_KBUILDSYCOCA NAMES kbuildsycoca6 kbuildsycoca5)
  set(GB_EMU_REFRESH_MENU_CACHES "")
  if(GB_EMU_UPDATE_DESKTOP_DATABASE)
    list(APPEND GB_EMU_REFRESH_MENU_CACHES COMMAND "${GB_EMU_UPDATE_DESKTOP_DATABASE}" "${GB_EMU_APPLICATIONS_DIR}")
  endif()
  if(GB_EMU_KBUILDSYCOCA)
    list(APPEND GB_EMU_REFRESH_MENU_CACHES COMMAND "${GB_EMU_KBUILDSYCOCA}")
  endif()

  # Opt-in, never part of a normal build:
  #   cmake --build out/build/Linux-GCC --target install-desktop-entry
  add_custom_target(install-desktop-entry
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GB_EMU_APPLICATIONS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/gb-emu-cpp.desktop" "${GB_EMU_APPLICATIONS_DIR}/gb-emu-cpp.desktop"
    ${GB_EMU_REFRESH_MENU_CACHES}
    COMMENT "Installing gb-emu-cpp.desktop into ${GB_EMU_APPLICATIONS_DIR}"
    VERBATIM
  )
  add_dependencies(install-desktop-entry gb-emu-cpp)
endif()