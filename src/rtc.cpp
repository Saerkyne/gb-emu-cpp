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
		live_rtc_registers.rtc_days_high = 0x80; // Set Day Counter Carry bit to 1, clear the day counter bit 8
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
	
	if (live_rtc_registers.rtc_seconds > 59) {
		live_rtc_registers.rtc_seconds = 0;
		rtc_tick_minutes();
	}
	else {
		live_rtc_registers.rtc_seconds++;
	}
}