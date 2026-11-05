#include "emulator_core.h"
#include "cart.h"
#include "cpu.h"
#include "interrupts.h"
#include "ppu.h"
#include "rtc.h"
#include "timer.h"
#include "memory_bus.h"
#include "joypad.h"
#include <chrono>
#include <thread>

const uint32_t CPU_FRAME_TIME_NS = 1.0 / 60.0 * 1000000000;
gfx_context* gfx_debug_ctx = nullptr;
gfx_context* gfx_render_ctx = nullptr;

uint32_t core_clock_counter = 0; // Global clock counter for the emulator core
bool core_quit_requested = false; // Flag to indicate if the emulator core should quit
std::chrono::steady_clock::time_point core_time_frame_begin;

void core_init_gfx_contexts();
void core_on_key_press(SDL_Scancode sym, bool down);
void core_sleep_until(std::chrono::steady_clock::time_point targetTime);

int core_init()
{

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return -1;
	}

	if (!cart_open_file()) 
	{
		return -1;
	}

	cart_print_info();

	core_init_gfx_contexts();

	ppu_init();
	timer_init();

	return 0;
}

void core_run() 
{
	memset(memory, 0, MEMORY_SIZE);
	cpu_reset();
	ppu_reset();

	core_time_frame_begin = std::chrono::steady_clock::now();

	while (!core_quit_requested) 
	{
		cpu_tick();
	}
}

void core_shutdown() 
{
	gfx_destroy(gfx_render_ctx);
	gfx_destroy(gfx_debug_ctx);
	
	cart_save_to_file(save_filename_buffer);

	SDL_Quit();

}

void core_on_frame()
{
	while (SDL_PollEvent(&gfx_render_ctx->event))
	{
		switch (gfx_render_ctx->event.type)
		{
		case SDL_EVENT_KEY_DOWN:
			core_on_key_press(gfx_render_ctx->event.key.scancode, true);
			break;
		case SDL_EVENT_KEY_UP:
			core_on_key_press(gfx_render_ctx->event.key.scancode, false);
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			core_quit_requested = true;
			break;
		case SDL_EVENT_QUIT:
			core_quit_requested = true;
			break;
		}
	}

	if (gfx_debug_ctx->window)
	{
		if (SDL_PollEvent(&gfx_debug_ctx->event) &&
			gfx_debug_ctx->event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		{
			core_quit_requested = true;
		}
	}

	auto time_frame_end = std::chrono::steady_clock::now();
	auto time_elapsed = time_frame_end - core_time_frame_begin;
	auto sleep_time = std::chrono::nanoseconds((CPU_FRAME_TIME_NS) - time_elapsed.count());
	if (sleep_time.count() > 0)
	{
		std::chrono::steady_clock::time_point end_of_sleep = time_frame_end + sleep_time;
		core_sleep_until(end_of_sleep);
	}
	core_time_frame_begin = std::chrono::steady_clock::now();
}

void core_advance_cpu_clocks(uint8_t clocks) 
{
	timer_advance_clocks(clocks);
	ppu_advance_clocks(clocks);
	rtc_advance_clocks(clocks);
	
	core_clock_counter += clocks;

	if (interrupt_enable_ime_delay > 0) 
	{
		for (uint8_t c = 0; c < clocks; c++) 
		{
			interrupt_enable_ime_delay--;
			if (interrupt_enable_ime_delay == 0) 
			{
				interrupt_master_enable = true;
				break;
			}
		}
	}
}

void core_init_gfx_contexts()
{
	const SDL_Color COLOR_BLACK = { 0, 0, 0, 0 };
	const int DEBUG_TEXTURE_SIZE = 512;

	gfx_debug_ctx = gfx_init(DEBUG_TEXTURE_SIZE, DEBUG_TEXTURE_SIZE, "Debug");
	gfx_clear(gfx_debug_ctx, COLOR_BLACK);
	gfx_update(gfx_debug_ctx);

	gfx_render_ctx = gfx_init(LCD_PIXELS_WIDTH * PPU_RENDER_SCALE, LCD_PIXELS_HEIGHT * PPU_RENDER_SCALE, "GoodBoy");
	gfx_clear(gfx_render_ctx, COLOR_BLACK);
	gfx_update(gfx_render_ctx);
}

void core_on_key_press(SDL_Scancode sym, bool down)
{
	switch (sym)
	{
	case SDL_SCANCODE_W:
		on_joypad_dpad_up(down);
		break;
	case SDL_SCANCODE_S:
		on_joypad_dpad_down(down);
		break;
	case SDL_SCANCODE_A:
		on_joypad_dpad_left(down);
		break;
	case SDL_SCANCODE_D:
		on_joypad_dpad_right(down);
		break;
	case SDL_SCANCODE_J:
		on_joypad_button_a(down);
		break;
	case SDL_SCANCODE_K:
		on_joypad_button_b(down);
		break;
	case SDL_SCANCODE_RETURN:
		on_joypad_button_start(down);
		break;
	case SDL_SCANCODE_RSHIFT:
		on_joypad_button_select(down);
		break;
	default:
		break;
	}
}

void core_sleep_until(std::chrono::steady_clock::time_point targetTime)
{
	auto now = std::chrono::steady_clock::now();
	auto buffer = std::chrono::milliseconds(2);

	if (targetTime - now > buffer)
	{
		std::this_thread::sleep_until(targetTime - buffer);
	}

	while (std::chrono::steady_clock::now() < targetTime)
	{
		std::this_thread::yield();
	}
}