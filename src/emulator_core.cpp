#include "emulator_core.h"
#include "cart.h"
#include "cpu.h"
#include "dma.h"
#include "interrupts.h"
#include "ppu.h"
#include "rtc.h"
#include "timer.h"
#include "memory_bus.h"
#include "joypad.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>

// Test Mode Block
bool test_mode = false;
int core_exit_code = CORE_EXIT_OK; // Exit code for the emulator core, used in test mode
const uint32_t CORE_TEST_CLOCK_LIMIT = 41194304u * 120u; // 120 seconds at 41,194,304 Hz
// End Test Mode Block

const uint32_t CPU_FRAME_TIME_NS = 1.0 / 60.0 * 1000000000;
gfx_context* gfx_debug_ctx = nullptr;
gfx_context* gfx_render_ctx = nullptr;

uint32_t core_clock_counter = 0; // Global clock counter for the emulator core
bool core_quit_requested = false; // Flag to indicate if the emulator core should quit
std::chrono::steady_clock::time_point core_time_frame_begin;

static char core_requested_rom_path[CART_MAX_PATH]; // Path of the ROM waiting to be loaded
static bool core_rom_load_pending = false; // Set by core_request_rom_load, cleared by core_handle_rom_request
static int core_files_in_current_drop = 0; // SDL_EVENT_DROP_FILE events seen since the last SDL_EVENT_DROP_BEGIN

void core_init_gfx_contexts();
void core_on_key_press(SDL_Scancode sym, bool down);
void core_sleep_until(std::chrono::steady_clock::time_point targetTime);
void core_poll_events();
void core_handle_rom_request();
void core_idle();

int core_init()
{

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return -1;
	}

	core_init_gfx_contexts();

	return 0;
}

void core_run() 
{
	core_time_frame_begin = std::chrono::steady_clock::now();

	while (!core_quit_requested) 
	{
		// Between cpu_tick calls no instruction is half-finished, so we can safely poll events and handle ROM requests here
		if (core_rom_load_pending) 
		{
			core_handle_rom_request();
		}

		if (cartridge_loaded) 
		{
			cpu_tick();

			// Test Mode Block
			if (test_mode && core_clock_counter >= CORE_TEST_CLOCK_LIMIT)
			{
				printf("NO RESULT (timed out)\n");
				core_quit_requested = true; 
			}
			// End Test Mode Block
		}
		// Test Mode Block
		else if (test_mode)
		{
			printf("NO ROM\n");
			core_exit_code = CORE_EXIT_NO_ROM;
			core_quit_requested = true;
		}
		// End Test Mode Block
		else 
		{
			core_idle();
		}
	}
}

void core_shutdown() 
{
	gfx_destroy(gfx_render_ctx);
	gfx_destroy(gfx_debug_ctx);
	
	cart_unload(); // Saves the game if it supports saves

	SDL_Quit();

}

void core_on_frame()
{
	core_poll_events();
	// Test Mode Block
	if (test_mode)
	{
		return; // In test mode, we don't want to sleep or wait for the next frame
	}
	// End Test Mode Block

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

void core_request_rom_load(const char* path)
{
	if (path == NULL || path[0] == '\0')
	{
		return;
	}

	const size_t path_length = strlen(path);
	if (path_length >= CART_MAX_PATH)
	{
		printf("ROM path is longer than %d characters and was not loaded: %s\n", CART_MAX_PATH - 1, path);
		return;
	}

	memcpy(core_requested_rom_path, path, path_length + 1); // + 1 copies the terminating null
	core_rom_load_pending = true; // Set last, so the path is complete before the request is visible
}

void core_poll_events()
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
		case SDL_EVENT_DROP_BEGIN: // A drag-and-drop operation has started; reset the counter for this operation
			core_files_in_current_drop = 0; // Reset the counter for this drop operation
			break;
		case SDL_EVENT_DROP_FILE: // One file per event; several files arrive as several events
			core_files_in_current_drop++;
			if (core_files_in_current_drop == 1) // Only load the first file in a multi-file drop
			{
				core_request_rom_load(gfx_render_ctx->event.drop.data); // Copies the path, with SDL frees after the event is processed
			}
			break;
		case SDL_EVENT_DROP_COMPLETE:
			if (core_files_in_current_drop > 1)
			{
				printf("Warning: %d files were dropped, but only the first one was loaded.\n", core_files_in_current_drop);
			}
			core_files_in_current_drop = 0; // Reset the counter for this drop operation
			break;
		default:
			break;
		}
	}
}

void core_handle_rom_request()
{
	core_rom_load_pending = false;

	// Check the new file before touching anything, so a bad file leaves the running game alone
	if (!cart_check_rom_file(core_requested_rom_path))
	{
		return; // cart_check_rom_file already printed the reason
	}

	// Take out the old cartridge: save it, then put everything cartridge-side back to power-on
	cart_unload();
	memory_bus_reset(); // MBC registers
	rtc_reset(); // Must come before cart_load, which can start the clock and load it from the save file
	core_clock_counter = 0; // Reset the global clock counter to zero, so the new cartridge starts at time zero
	
	joypad_reset();

	if (!cart_load(core_requested_rom_path))
	{
		return; // Rare after the check above (a read error, for example); the emulator is now idle
	}

	// Put the machine in its initial state after loading a new ROM. memset first, so PPU and timer work correctly
	memset(memory, 0, MEMORY_SIZE);
	cpu_reset();
	ppu_init();
	ppu_reset();
	timer_init();
	dma_reset();

	cart_print_info();

	core_time_frame_begin = std::chrono::steady_clock::now(); // Reset the frame timer after loading a new ROM
}

void core_idle()
{
	core_poll_events();
	std::this_thread::sleep_for(std::chrono::milliseconds(16)); // About 60 polls per second while there is no ROM loaded
}

void core_advance_cpu_clocks(uint8_t clocks) 
{
	timer_advance_clocks(clocks);
	ppu_advance_clocks(clocks);
	rtc_advance_clocks(clocks);
	dma_advance_clocks();
	
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

// Test Mode Block
void core_report_test_result(bool passed)
{
	if (core_quit_requested)
	{
		return; // Don't overwrite the exit code if the user requested a quit. Mooneye tests execute LD B,B twice; only first set counts
	}
	printf("%s\n", passed ? "PASS" : "FAIL");
	core_exit_code = passed ? CORE_EXIT_OK : CORE_EXIT_TEST_FAILED;
	core_quit_requested = true;
}
// End Test Mode Block