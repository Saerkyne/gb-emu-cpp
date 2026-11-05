#pragma once
#include <stdint.h>

struct rtc_clock_registers {
	uint8_t rtc_seconds;   // RTC seconds register
	uint8_t rtc_minutes;   // RTC minutes register
	uint8_t rtc_hours;     // RTC hours register
	uint8_t rtc_days_low;  // RTC days low register
	uint8_t rtc_days_high; // RTC days high register
};

extern uint32_t rtc_subsecond_counter; // counts towards 4194304 ticks (1 second) for the RTC
extern bool cart_has_rtc; // indicates if the cartridge has an RTC
extern uint8_t rtc_previous_latch_write; // stores the previous value written to the latch register (0x6000-0x7FFF) to detect the latch command sequence

void rtc_init(); // zeros out the RTC registers and sets up the initial state and sets the cart_has_rtc flag
void rtc_advance_clocks(const uint8_t cycles); // advances subsecond counter and calls rtc_tick when it fills
void rtc_register_write(const uint16_t selector, const uint8_t value); // handles writes to the RTC registers
uint8_t rtc_register_read(const uint16_t selector); // handles reads from the RTC registers
void rtc_tick(); // increments the RTC registers, handling overflow and day counter wraparound
void rtc_latch_write(const uint8_t value); // handles writes to the latch register, detecting the latch command sequence and latching the RTC registers
void rtc_catch_up(const uint64_t seconds); // catches up the RTC registers by the specified number of seconds, handling overflow and day counter wraparound
void rtc_create_save_bytes(uint8_t* save_bytes); // creates a byte array representing the current state of the RTC registers for saving to a file
void rtc_load_save_bytes(const uint8_t* save_bytes); // loads the RTC registers from a byte array representing the saved state of the RTC registers