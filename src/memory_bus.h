#pragma once

#include <stdint.h>

#define CLEAR_BIT(number, bit)        number &= ~(1UL << bit)
#define CHECK_BIT(number, bit)        ((number >> bit) & 1U)
#define CHANGE_BIT(number, value, bit) number ^= (-value ^ number) & (1UL << bit)
#define BIT(n)  (1 << (n))

const uint16_t ADDR_IO_IF = 0xFF0F; // Interrupt Flag
const uint16_t ADDR_IO_IE = 0xFFFF; // Interrupt Enable

constexpr uint32_t MEMORY_SIZE = 64 * 1024; // 64 KB of memory
constexpr uint32_t ERAM_SIZE = 8 * 16 * 1024; // 16 banks of 8KiB

extern uint8_t memory[MEMORY_SIZE];
extern uint8_t eram[ERAM_SIZE];

uint8_t memory_bus_read(const uint16_t addr);
void memory_bus_write(const uint16_t addr, const uint8_t value);
void memory_bus_reset();

// Might be worth adding a function here: for MBC3 read/write. some duplicated code for 
// checking if RAM is enabled, and if the bank number is valid. Could be a helper function to reduce code duplication.