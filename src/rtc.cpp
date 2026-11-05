#include "rtc.h"

rtc_clock_registers live_rtc_registers;
rtc_clock_registers latch_rtc_registers;
uint32_t rtc_subsecond_counter; // counts towards 4194304 ticks (1 second) for the RTC
uint8_t rtc_previous_latch_write; // detects the state of the latch command sequence (0x00 -> 0x01 in 0x6000-0x7FFF)

bool  cart_has_rtc; // indicates if the cartridge has an RTC

void rtc_tick_days_high() {
	uint8_t curr_days_high = live_rtc_registers.rtc_days_high; // read the current days high value
	if (curr_days_high & 0b00000001) // Check if day counter bit 8 is set
	{
		if (curr_days_high & 0b10000000) // Check if Day Counter Carry bit is set
		{
			live_rtc_registers.rtc_days_high = curr_days_high ^ 0b00000001; // Clear day counter bit 8, carry flag is sticky until cleared intentionally
		}
		else
		{
			live_rtc_registers.rtc_days_high = curr_days_high ^ 0b10000001; // Clear day counter bit 8 but set the Day Counter Carry bit
		}
	}
	else
	{
		live_rtc_registers.rtc_days_high = curr_days_high + 1; // Increment days high
	}
}

void rtc_tick_days_low() {
	uint8_t curr_days_low = live_rtc_registers.rtc_days_low; // read the current days low value
	if (curr_days_low < 255) // Check for days low overflow
	{
		live_rtc_registers.rtc_days_low++; // Increment days low
	}
	else if (curr_days_low == 255)
	{
		live_rtc_registers.rtc_days_low = 0; // Reset days low to 0
		rtc_tick_days_high(); // Increment days high
	}
}

void rtc_tick_hours() {
	uint8_t curr_hours = live_rtc_registers.rtc_hours; // read the current hours value
	if (curr_hours < 23) // Check for hours overflow
	{
		live_rtc_registers.rtc_hours++; // Increment hours
	}
	else if (curr_hours == 23)
	{
		live_rtc_registers.rtc_hours = 0; // Reset hours to 0
		rtc_tick_days_low(); // Increment days low
	}
	else if (curr_hours >= 24 && curr_hours <= 30)
	{
		live_rtc_registers.rtc_hours++; // Increment hours, do not increment days low
	}
	else if (curr_hours == 31)
	{
		live_rtc_registers.rtc_hours = 0; // Reset hours to 0, do not increment days low
	}
}

void rtc_tick_minutes() {
	uint8_t curr_minutes = live_rtc_registers.rtc_minutes; // read the current minutes value
	if (curr_minutes < 59) // Check for minutes overflow
	{
		live_rtc_registers.rtc_minutes++; // Increment minutes
	}
	else if (curr_minutes == 59)
	{
		live_rtc_registers.rtc_minutes = 0; // Reset minutes to 0
		rtc_tick_hours(); // Increment hours
	}
	else if (curr_minutes >= 60 && curr_minutes <= 62)
	{
		live_rtc_registers.rtc_minutes++; // Increment minutes, do not increment hours
	}
	else if (curr_minutes == 63)
	{
		live_rtc_registers.rtc_minutes = 0; // Reset minutes to 0, do not increment hours
	}
}

void rtc_tick_seconds() {
	uint8_t curr_seconds = live_rtc_registers.rtc_seconds; // read the current seconds value
	if (curr_seconds < 59) // Check for seconds overflow
	{
		live_rtc_registers.rtc_seconds++; // Increment seconds
	}
	else if (curr_seconds == 59)
	{
		live_rtc_registers.rtc_seconds = 0; // Reset seconds to 0
		rtc_tick_minutes(); // Increment minutes
	}
	else if (curr_seconds >= 60 && curr_seconds <= 62)
	{
		live_rtc_registers.rtc_seconds++; // Increment seconds, do not increment minutes
	}
	else if (curr_seconds == 63)
	{
		live_rtc_registers.rtc_seconds = 0; // Reset seconds to 0, do not increment minutes
	}

}

void rtc_tick() {
	if (!cart_has_rtc) {
		return; // If the cartridge doesn't have an RTC, do nothing
	}
	
	rtc_tick_seconds(); // Increment seconds, which will cascade to minutes, hours, and days as needed
}

void rtc_advance_clocks(const uint8_t cycles) {
	if (!cart_has_rtc) 
	{
		return; // If the cartridge doesn't have an RTC, do nothing
	}

	if (live_rtc_registers.rtc_days_high & 0b01000000) 
	{ 
		return; // If the RTC is halted, do not advance the RTC clock
	}

	rtc_subsecond_counter += cycles; // Increment the subsecond counter by the number of cycles
	if (rtc_subsecond_counter >= 4194304) // Check if the subsecond counter has reached 1 second (4194304 cycles)
	{
		rtc_subsecond_counter = rtc_subsecond_counter - 4194304; // Reset the subsecond counter by subtracting 4194304
		rtc_tick(); // Increment the RTC registers
	}
}

void rtc_init() {
	
	// Initialize the RTC registers and subsecond counter
	live_rtc_registers.rtc_seconds = 0;
	live_rtc_registers.rtc_minutes = 0;
	live_rtc_registers.rtc_hours = 0;
	live_rtc_registers.rtc_days_low = 0;
	live_rtc_registers.rtc_days_high = 0;
	rtc_subsecond_counter = 0;
	cart_has_rtc = true;
}