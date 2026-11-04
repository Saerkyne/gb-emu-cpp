#include "rtc.h"

live_rtc_clock_registers* live_rtc_registers;
latch_rtc_clock_registers* latch_rtc_registers;
uint16_t rtc_tick_internal_sysclk; // counts towards 4194304 ticks (1 second) for the RTC
uint8_t rtc_latch_state; // detects the state of the latch command sequence (0x00 -> 0x01 in 0x6000-0x7FFF)


