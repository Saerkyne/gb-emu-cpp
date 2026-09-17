#pragma once

#include <stdint.h>

const uint8_t INTERRUPT_FLAG_VBLANK = (1 << 0);
const uint8_t INTERRUPT_FLAG_STAT = (1 << 1);
const uint8_t INTERRUPT_FLAG_TIMER = (1 << 2);
const uint8_t INTERRUPT_FLAG_SERIAL = (1 << 3);
const uint8_t INTERRUPT_FLAG_JOYPAD = (1 << 4);

extern bool interrupt_master_enable;
extern uint8_t interrupt_enable_ime_delay;

void interrupt_enable_write(uint8_t value);
void interrupt_flag_write(uint8_t value);

void interrupt_raise_flag(uint8_t flag);

// called by CPU
void interrupt_service_routine();