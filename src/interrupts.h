#pragma once

#include <stdint.h>

const uint8_t INTERRUPT_FLAG_VBLANK = (1 << 0);
const uint8_t INTERRUPT_FLAG_STAT = (1 << 1);
const uint8_t INTERRUPT_FLAG_TIMER = (1 << 2);
const uint8_t INTERRUPT_FLAG_SERIAL = (1 << 3);
const uint8_t INTERRUPT_FLAG_JOYPAD = (1 << 4);

const uint16_t INTERRUPT_HANDLER_V_BLANK = 0x0040;
const uint16_t INTERRUPT_HANDLER_LCD_STATUS = 0x0048;
const uint16_t INTERRUPT_HANDLER_TIMER = 0x0050;
const uint16_t INTERRUPT_HANDLER_SERIAL = 0x0058;
const uint16_t INTERRUPT_HANDLER_JOYPAD = 0x0060;

extern bool interrupt_master_enable;
extern uint8_t interrupt_enable_ime_delay;
extern bool interrupt_stat_signal;

void interrupt_enable_write(uint8_t value);
void interrupt_flag_write(uint8_t value);

void interrupt_raise_flag(uint8_t flag);

// called by CPU
void interrupt_service_routine();