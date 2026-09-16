cmake_minimum_required(VERSION 4.3)
project(gb_emu_cpp)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(gb_emu_cpp
        cart.cpp
        cart.h
        cpu.cpp
        cpu.h
        cpu_instructions.cpp
        cpu_instructions.h
        cpu_routines.h
        emulator_core.cpp
        emulator_core.h
        main.cpp
        memory_bus.cpp
        memory_bus.h
        resource.h)
