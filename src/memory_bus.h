#pragma once

#include <stdint.h>

#define CLEAR_BIT(number, bit)        number &= ~(1UL << bit)
#define CHECK_BIT(number, bit)        ((number >> bit) & 1U)
#define BIT(n)  (1 << (n))

const uint16_t ADDR_IO_IF = 0xFF0F; // Interrupt Flag
const uint16_t ADDR_IO_IE = 0xFFFF; // Interrupt Enable

constexpr uint32_t MEMORY_SIZE = 64 * 1024; // 64 KB of memory
constexpr uint32_t ERAM_SIZE = 8 * 4 * 1024;

extern uint8_t memory[MEMORY_SIZE];

uint8_t memory_bus_read(const uint16_t addr);
void memory_bus_write(const uint16_t addr, const uint8_t value);