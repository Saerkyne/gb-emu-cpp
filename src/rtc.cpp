#include "rtc.h"

rtc_clock_registers live_rtc_registers;
rtc_clock_registers latch_rtc_registers;
uint32_t rtc_subsecond_counter; // counts towards 4194304 ticks (1 second) for the RTC
uint8_t rtc_latch_state; // detects the state of the latch command sequence (0x00 -> 0x01 in 0x6000-0x7FFF)

bool  cart_has_rtc; // indicates if the cartridge has an RTC

void rtc_tick_days_high() {
	uint8_t prev_days_high = rtc_register_read(0x0C); // read the previous days high value
	if (prev_days_high & 0b00000001) // Check if day counter overflow bit is set
	{
		rtc_register_write(0x0C, prev_days_high | 0b10000000); // Set Day Counter Carry bit to 1
	}
	else
	{
		rtc_register_write(0x0C, prev_days_high + 1); // Increment days high
	}
}

void rtc_tick_days_low() {
	uint8_t prev_days_low = rtc_register_read(0x0B); // read the previous days low value
	if (prev_days_low < 255) // Check for days low overflow
	{
		rtc_register_write(0x0B, prev_days_low + 1); // Increment days low
	}
	else if (prev_days_low == 255)
	{
		rtc_register_write(0x0B, 0); // Reset days low to 0
		rtc_tick_days_high(); // Increment days high
	}
}

void rtc_tick_hours() {
	uint8_t prev_hours = rtc_register_read(0x0A); // read the previous hours value
	if (prev_hours < 23) // Check for hours overflow
	{
		rtc_register_write(0x0A, prev_hours + 1); // Increment hours
	}
	else if (prev_hours == 23)
	{
		rtc_register_write(0x0A, 0); // Reset hours to 0
		rtc_tick_days_low(); // Increment days low
	}
	else if (prev_hours >= 24 && prev_hours <= 31)
	{
		rtc_register_write(0x0A, 0); // Reset hours to 0, do not increment days low
	}
}

void rtc_tick_minutes() {
	uint8_t prev_minutes = rtc_register_read(0x09); // read the previous minutes value
	if (prev_minutes < 59) // Check for minutes overflow
	{
		rtc_register_write(0x09, prev_minutes + 1); // Increment minutes
	}
	else if (prev_minutes == 59)
	{
		rtc_register_write(0x09, 0); // Reset minutes to 0
		rtc_tick_hours(); // Increment hours
	}
	else if (prev_minutes >= 60 && prev_minutes <= 63)
	{
		rtc_register_write(0x09, 0); // Reset minutes to 0, do not increment hours
	}
}

void rtc_tick_seconds() {
	uint8_t prev_seconds = rtc_register_read(0x08); // read the previous seconds value
	if (prev_seconds < 59) // Check for seconds overflow
	{
		rtc_register_write(0x08, prev_seconds + 1); // Increment seconds
	}
	else if (prev_seconds == 59)
	{
		rtc_register_write(0x08, 0); // Reset seconds to 0
		rtc_tick_minutes(); // Increment minutes
	}
	else if (prev_seconds >= 60 && prev_seconds <= 63)
	{
		rtc_register_write(0x08, 0); // Reset seconds to 0, do not increment minutes
	}
}

void rtc_tick() {
	if (!cart_has_rtc) {
		return; // If the cartridge doesn't have an RTC, do nothing
	}

	uint8_t prev_days_high = rtc_register_read(0x0C); // read the previous days high value
	if (!(prev_days_high & 0b01000000)) // Check if halt bit is not set (RTC is running)
	{
		rtc_register_write(0x0C, prev_days_high | 0b01000000); // Set Halt bit to 1 to stop the RTC from ticking while we update the registers
		rtc_tick_seconds(); // Increment seconds
		
	}
}