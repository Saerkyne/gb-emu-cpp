#pragma once

#include <stdint.h>

uint8_t memory_bus_read(const uint16_t addr);
void memory_bus_write(const uint16_t addr, const uint8_t value);