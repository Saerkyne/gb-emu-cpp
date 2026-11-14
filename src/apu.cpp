#include "apu.h"
#include "memory_bus.h"
#include "debug_log.h"
#include "emulator_core.h"
#include <SDL3/SDL.h>

const uint32_t APU_SAMPLE_RATE = 48000;
const uint32_t APU_SAMPLES_PER_FRAME = 17556;
const uint32_t APU_OUT_SAMPLES_PER_FRAME = (APU_SAMPLE_RATE / 60);
const float APU_OUT_DIVISOR = (float)APU_OUT_SAMPLES_PER_FRAME / APU_SAMPLES_PER_FRAME;

#if CORE_RENDER_APU_DEBUG
#include "ppu.h"
void render_debug_output_waves(void* pixels);
void render_debug_channels(void* pixels);
void apu_render_debug();
uint8_t sq1_sample_buffer[APU_SAMPLES_PER_FRAME];
uint8_t sq2_sample_buffer[APU_SAMPLES_PER_FRAME];
uint8_t wave_sample_buffer[APU_SAMPLES_PER_FRAME];
uint8_t noise_sample_buffer[APU_SAMPLES_PER_FRAME];
uint32_t raw_buffer_index = 0;
#endif

uint8_t sq1_sample_latest = 0;
uint8_t sq2_sample_latest = 0;
uint8_t wave_sample_latest = 0;
uint8_t noise_sample_latest = 0;
uint32_t apu_out_sample_index = 0;
float apu_out_samples[APU_OUT_SAMPLES_PER_FRAME * 2];
float apu_out_accumulator = 0.f;

SDL_AudioStream* apu_stream = NULL;

audio_register_set* audio_registers = (audio_register_set*)(memory + 0xFF10);

const uint8_t square_duty_cycles[32] = {
	0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 1, 1,
	0, 1, 1, 1, 1, 1, 1, 0
};

const uint8_t wave_volume_shifts[4] = { 4, 0, 1, 2 };

const uint8_t noise_divisors[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

const uint8_t read_or_table_nr1[5] = { 0x80, 0x3F, 0x00, 0xFF, 0xBF };
const uint8_t read_or_table_nr2[5] = { 0xFF, 0x3F, 0x00, 0xFF, 0xBF };
const uint8_t read_or_table_nr3[5] = { 0x7F, 0xFF, 0x9F, 0xFF, 0xBF };
const uint8_t read_or_table_nr4[5] = { 0xFF, 0xFF, 0x00, 0x00, 0xBF };
const uint8_t read_or_table_nr5[3] = { 0x00, 0x00, 0x70 };

uint8_t apu_clock_counter = 0;

uint16_t sq1_freq_counter = 0;
uint8_t sq1_length_counter = 0;
uint8_t sq1_phase = 0;
uint8_t sq1_env_volume = 0;
uint8_t sq1_env_counter = 0;
uint8_t sq1_sweep_counter = 0;
uint16_t sq1_sweep_frequency = 0;
bool sq1_sweep_enabled = false;

uint16_t sq2_freq_counter = 0;
uint8_t sq2_length_counter = 0;
uint8_t sq2_phase = 0;
uint8_t sq2_env_volume = 0;
uint8_t sq2_env_counter = 0;

uint16_t wave_freq_counter = 0;
uint16_t wave_length_counter = 0;
uint8_t wave_position = 0;

uint8_t wave_table[16] = { 0x84, 0x40, 0x43, 0xAA, 0x2D, 0x78, 0x92, 0x3C, 0x60, 0x59, 0x59, 0xB0, 0x2E, 0xDA };

uint32_t noise_freq_counter = 0;
uint8_t noise_length_counter = 0;
uint16_t noise_prng = 1;
uint8_t noise_env_volume = 0;
uint8_t noise_env_counter = 0;

float lpf_alpha = 0.f;
float prev_sampleL = 0.f;
float prev_sampleR = 0.f;

void tick_noise_lfsr();
float lpf_calculate_alpha(float cutoff_freq, float sample_rate);
float lpf_process_sample(float input, float alpha, float prev_output);

void apu_init()
{
	SDL_AudioSpec spec;
	spec.channels = 2;
	spec.format = SDL_AUDIO_F32;
	spec.freq = APU_SAMPLE_RATE;
	apu_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
	if (!apu_stream)
	{
		debug_log("APU Failed to create SDL audio stream\n");
		return;
	}
	SDL_ResumeAudioStreamDevice(apu_stream);

	lpf_alpha = lpf_calculate_alpha(20000.f, (float)APU_SAMPLE_RATE);
}

void apu_reset()
{
	apu_clock_counter = 0;
	audio_registers->nr1[0] = 0;
	audio_registers->nr1[1] = 0xBF;
	audio_registers->nr1[2] = 0xF3;
	audio_registers->nr1[3] = 0xC1;
	audio_registers->nr1[4] = 0x07;
	audio_registers->nr2[0] = 0;
	audio_registers->nr2[1] = 0;
	audio_registers->nr2[2] = 0;
	audio_registers->nr2[3] = 0;
	audio_registers->nr2[4] = 0;
	audio_registers->nr3[0] = 0;
	audio_registers->nr3[1] = 0;
	audio_registers->nr3[2] = 0;
	audio_registers->nr3[3] = 0;
	audio_registers->nr3[4] = 0;
	audio_registers->nr4[0] = 0;
	audio_registers->nr4[1] = 0;
	audio_registers->nr4[2] = 0;
	audio_registers->nr4[3] = 0;
	audio_registers->nr4[4] = 0;
	audio_registers->nr5[0] = 0x77;
	audio_registers->nr5[1] = 0xF3;
	audio_registers->nr5[2] = 0x81;
}

void apu_tick(uint8_t clocks)
{
	for (uint8_t i = 0; i < clocks; i++)
	{
		sq1_freq_counter -= 1;
		sq2_freq_counter -= 1;
		wave_freq_counter -= 1;
		noise_freq_counter -= 1;
		
		if (sq1_freq_counter == 0)
		{
			sq1_freq_counter = 4 * (2048 - SQ1_FREQUENCY);
			sq1_phase = (sq1_phase + 1) % 8;
		}
		if (sq2_freq_counter == 0)
		{
			sq2_freq_counter = 4 * (2048 - SQ2_FREQUENCY);
			sq2_phase = (sq2_phase + 1) % 8;
		}
		if (wave_freq_counter == 0)
		{
			wave_freq_counter = 2 * (2048 - WAVE_FREQUENCY);
			wave_position = (wave_position + 1) % 32;
		}
		if (noise_freq_counter == 0)
		{
			noise_freq_counter = noise_divisors[NOISE_DIVISOR_CODE] * (1 << NOISE_CLOCK_SHIFT + 1);
			tick_noise_lfsr();
		}

		if (i % 4 == 0) // 1Mhz
		{
			if (APU_STATUS_SQ1)
			{
				const uint8_t duty_index = (SQ1_DUTY * 8) + sq1_phase;
				sq1_sample_latest = square_duty_cycles[duty_index] * sq1_env_volume;
			}
			else
			{
				sq1_sample_latest = 0;
			}
			if (APU_STATUS_SQ2)
			{
				const uint8_t duty_index = (SQ2_DUTY * 8) + sq2_phase;
				sq2_sample_latest = square_duty_cycles[duty_index] * sq2_env_volume;
			}
			else
			{
				sq2_sample_latest = 0;
			}
			if (APU_STATUS_WAVE && WAVE_DAC_POWER)
			{
				const bool low_nibble = wave_position % 2;
				const uint8_t wave_sample = low_nibble ? wave_table[wave_position / 2] & 0b00001111 : wave_table[wave_position / 2] >> 4;
				const uint8_t vol_shift = wave_volume_shifts[WAVE_VOLUME_CODE];
				wave_sample_latest = wave_sample >> vol_shift;
			}
			else
			{
				wave_sample_latest = 0;
			}
			if (APU_STATUS_NOISE)
			{
				const bool value = noise_prng & 0b0000000000000001;
				noise_sample_latest = value * noise_env_volume;
			}
			else
			{
				noise_sample_latest = 0;
			}
#if CORE_RENDER_APU_DEBUG
			sq1_sample_buffer[raw_buffer_index] = sq1_sample_latest;
			sq2_sample_buffer[raw_buffer_index] = sq2_sample_latest;
			wave_sample_buffer[raw_buffer_index] = wave_sample_latest;
			noise_sample_buffer[raw_buffer_index] = noise_sample_latest;
			raw_buffer_index++;
#endif

			apu_out_accumulator += APU_OUT_DIVISOR;
			if (apu_out_accumulator >= 1.f)
			{
				apu_out_accumulator -= 1.f;
				float sampleL = 0.f;
				float sampleR = 0.f;

				const float sq1_sample = TO_FLOAT_SAMPLE(sq1_sample_latest);
				const float sq2_sample = TO_FLOAT_SAMPLE(sq2_sample_latest);
				const float wave_sample = TO_FLOAT_SAMPLE(wave_sample_latest);
				const float noise_sample = TO_FLOAT_SAMPLE(noise_sample_latest);

				if (APU_ENABLE_SQ1_LEFT) sampleL += sq1_sample;
				if (APU_ENABLE_SQ1_RIGHT) sampleR += sq1_sample;
				if (APU_ENABLE_SQ2_LEFT) sampleL += sq2_sample;
				if (APU_ENABLE_SQ2_RIGHT) sampleR += sq2_sample;
				if (APU_ENABLE_WAVE_LEFT) sampleL += wave_sample;
				if (APU_ENABLE_WAVE_RIGHT) sampleR += wave_sample;
				if (APU_ENABLE_NOISE_LEFT) sampleL += noise_sample;
				if (APU_ENABLE_NOISE_RIGHT) sampleR += noise_sample;

				const float volL = (float)APU_VOLUME_LEFT / 7.f;
				const float volR = (float)APU_VOLUME_RIGHT / 7.f;
				apu_out_samples[apu_out_sample_index++] = (sampleL * volL / 4.f);
				apu_out_samples[apu_out_sample_index++] = (sampleR * volR / 4.f);

				if (apu_out_sample_index == 1600)
				{
					apu_on_frame_end();
					apu_out_sample_index = 0;
				}
			}
		}

	}
}

void apu_shutdown()
{
	SDL_DestroyAudioStream(apu_stream);
	apu_stream = NULL;
}

void apu_on_frame_end()
{
#if CORE_RENDER_APU_DEBUG
	apu_render_debug();
	raw_buffer_index = 0;
#endif

	for (uint16_t i = 0; i < APU_OUT_SAMPLES_PER_FRAME; i++)
	{
		const float sampleL = apu_out_samples[i * 2];
		const float sampleR = apu_out_samples[i * 2 + 1];
		prev_sampleL = lpf_process_sample(sampleL, lpf_alpha, prev_sampleL);
		prev_sampleR = lpf_process_sample(sampleR, lpf_alpha, prev_sampleR);
		apu_out_samples[i * 2] = prev_sampleL;
		apu_out_samples[i * 2 + 1] = prev_sampleR;
	}
	SDL_PutAudioStreamData(apu_stream, apu_out_samples, sizeof(float) * apu_out_sample_index);
}

void apu_clock_sq1_length()
{
	sq1_length_counter--;
	SET_SQ1_LENGTH_LOAD(sq1_length_counter);
	if (sq1_length_counter == 0)
	{
		SET_APU_STATUS_SQ1(0);
	}
}

void apu_clock_sq2_length()
{
	sq2_length_counter--;
	SET_SQ2_LENGTH_LOAD(sq2_length_counter);
	if (sq2_length_counter == 0)
	{
		SET_APU_STATUS_SQ2(0);
	}
}

void apu_clock_wave_length()
{
	wave_length_counter--;
	SET_WAVE_LENGTH_LOAD(wave_length_counter);
	if (wave_length_counter == 0)
	{
		SET_APU_STATUS_WAVE(0);
	}
}

void apu_clock_noise_length()
{
	noise_length_counter--;
	SET_NOISE_LENGTH_LOAD(noise_length_counter);
	if (noise_length_counter == 0)
	{
		SET_APU_STATUS_NOISE(0);
	}
}

void calculate_sq1_sweep(bool write)
{
	int16_t freq_sweep = sq1_sweep_frequency >> SQ1_SWEEP_SHIFT;
	if (SQ1_SWEEP_NEGATE)
	{
		freq_sweep *= -1;
	}
	const uint16_t new_frequency = sq1_sweep_frequency + freq_sweep;
	if (new_frequency <= 0x7FF)
	{
		if (write && SQ1_SWEEP_SHIFT != 0)
		{
			sq1_sweep_frequency = new_frequency;
			audio_registers->nr1[3] = new_frequency & 0xFF; // Frequency LSB write
			audio_registers->nr1[4] &= 0b11111000;			// Frequency MSB clear
			audio_registers->nr1[4] = new_frequency >> 8;	// Frequency MSB write
		}
	}
	else // sweep overflow
	{
		SET_APU_STATUS_SQ1(0);
	}
}

void apu_counter_tick()
{
	apu_clock_counter = (apu_clock_counter + 1) % 8;
	if (apu_clock_counter % 2 == 0)
	{
		if (SQ1_LENGTH_ENABLE && sq1_length_counter > 0)
		{
			apu_clock_sq1_length();
		}
		if (SQ2_LENGTH_ENABLE && sq2_length_counter > 0)
		{
			apu_clock_sq2_length();
		}
		if (WAVE_LENGTH_ENABLE && wave_length_counter > 0)
		{
			apu_clock_wave_length();
		}
		if (NOISE_LENGTH_ENABLE && noise_length_counter > 0)
		{
			apu_clock_noise_length();
		}
	}
	if (APU_STATUS_SQ1)
	{
		if (apu_clock_counter == 7 && SQ1_ENV_PERIOD > 0)
		{
			sq1_env_counter -= 1;
			if (sq1_env_counter == 0)
			{
				sq1_env_counter = SQ1_ENV_PERIOD;
				if (SQ1_ENV_ADD && sq1_env_volume < 15)
				{
					sq1_env_volume += 1;
				}
				else if (!SQ1_ENV_ADD && sq1_env_volume > 0)
				{
					sq1_env_volume -= 1;
				}
			}
		}
		if ((apu_clock_counter == 2 || apu_clock_counter == 6) &&
			sq1_sweep_enabled && SQ1_SWEEP_PERIOD != 0)
		{
			sq1_sweep_counter--;
			if (sq1_sweep_counter == 0)
			{
				sq1_sweep_counter = SQ1_SWEEP_PERIOD;
				calculate_sq1_sweep(true);
				calculate_sq1_sweep(false);
			}
		}
	}
	if (APU_STATUS_SQ2)
	{
		if (apu_clock_counter == 7 && SQ2_ENV_PERIOD > 0)
		{
			sq1_env_counter -= 1;
			if (sq1_env_counter == 0)
			{
				sq1_env_counter = SQ2_ENV_PERIOD;
				if (SQ2_ENV_ADD && sq2_env_volume < 15)
				{
					sq2_env_volume += 1;
				}
				else if (!SQ2_ENV_ADD && sq2_env_volume > 0)
				{
					sq2_env_volume -= 1;
				}
			}
		}
	}
	if (APU_STATUS_NOISE)
	{
		if (apu_clock_counter == 7 && NOISE_ENV_PERIOD > 0)
		{
			noise_env_counter -= 1;
			if (noise_env_counter == 0)
			{
				noise_env_counter = NOISE_ENV_PERIOD;
				if (NOISE_ENV_ADD && noise_env_volume < 15)
				{
					noise_env_volume += 1;
				}
				else if (!NOISE_ENV_ADD && noise_env_volume > 0)
				{
					noise_env_volume -= 1;
				}
			}
		}
	}
}

uint8_t apu_read(uint16_t addr)
{
	if (addr >= 0xFF10 && addr <= 0xFF14) // Square 1
	{
		const uint16_t reg_idx = addr - 0xFF10;
		return audio_registers->nr1[reg_idx] | read_or_table_nr1[reg_idx];
	}
	else if (addr >= 0xFF15 && addr <= 0xFF19) // Square 2
	{
		const uint16_t reg_idx = addr - 0xFF15;
		return audio_registers->nr2[reg_idx] | read_or_table_nr2[reg_idx];
	}
	else if (addr >= 0xFF1A && addr <= 0xFF1E) // Wave
	{
		const uint16_t reg_idx = addr - 0xFF1A;
		return audio_registers->nr3[reg_idx] | read_or_table_nr3[reg_idx];
	}
	else if (addr >= 0xFF1F && addr <= 0xFF23) // Noise
	{
		const uint16_t reg_idx = addr - 0xFF1F;
		return audio_registers->nr4[reg_idx] | read_or_table_nr4[reg_idx];
	}
	else if (addr >= 0xFF24 && addr <= 0xFF26) // Control
	{
		const uint16_t reg_idx = addr - 0xFF24;
		return audio_registers->nr5[reg_idx] | read_or_table_nr5[reg_idx];
	}
	else if (addr >= 0xFF30 && addr <= 0xFF3F) // Wave Table
	{
		const uint16_t samples_idx = addr - 0xFF30;
		return wave_table[samples_idx];
	}

	return 0xFF;
}

void apu_write(uint16_t addr, uint8_t value)
{
	if (addr >= 0xFF30 && addr <= 0xFF3F) // Wave Table is always writeable
	{
		const uint16_t samples_idx = addr - 0xFF30;
		wave_table[samples_idx] = value;
	}
	if (!APU_POWER_STATUS && addr != 0xFF26)
	{
		return;
	}

	// Square 1
	if (addr == 0xFF10)
	{
		audio_registers->nr1[0] = value;
	}
	else if (addr == 0xFF11)
	{
		audio_registers->nr1[1] = value;
		if (SQ1_LENGTH_LOAD == 0)
		{
			sq1_length_counter = 64;
			SET_SQ1_LENGTH_LOAD(63);
		}
		else
		{
			sq1_length_counter = 64 - (value & 0b00111111);
			SET_SQ1_LENGTH_LOAD(sq1_length_counter);
		}
	}
	else if (addr == 0xFF12)
	{
		audio_registers->nr1[2] = value;
		sq1_env_volume = SQ1_ENV_VOLUME;
		sq1_env_counter = SQ1_ENV_PERIOD > 0 ? SQ1_ENV_PERIOD : 8;
		if (SQ1_ENV_VOLUME == 0)
		{
			SET_APU_STATUS_SQ1(0);
		}
	}
	else if (addr == 0xFF13)
	{
		audio_registers->nr1[3] = value;
	}
	else if (addr == 0xFF14)
	{
		const bool length_enable_prev = SQ1_LENGTH_ENABLE;
		audio_registers->nr1[4] = value;
		const bool length_became_enabled = !length_enable_prev && SQ1_LENGTH_ENABLE;
		const bool is_first_half_of_frame_clock = apu_clock_counter % 2 == 0;
		const bool triggered = SQ1_TRIGGER;
		if (triggered)
		{
			SET_SQ1_TRIGGER(0);
			sq1_freq_counter = 4 * (2048 - SQ1_FREQUENCY);
			sq1_env_volume = SQ1_ENV_VOLUME;
			sq1_env_counter = SQ1_ENV_PERIOD > 0 ? SQ1_ENV_PERIOD : 8;
			sq1_sweep_frequency = SQ1_FREQUENCY;
			sq1_sweep_counter = SQ1_SWEEP_PERIOD > 0 ? SQ1_SWEEP_PERIOD : 8;
			sq1_sweep_enabled = (SQ1_SWEEP_PERIOD != 0 || SQ1_SWEEP_SHIFT != 0);
			if (SQ1_SWEEP_SHIFT != 0)
			{
				calculate_sq1_sweep(false);
			}
			if (SQ1_LENGTH_LOAD == 0)
			{
				sq1_length_counter = 64;
				SET_SQ1_LENGTH_LOAD(63);
			}

			const bool sq1_volume_enable = SQ1_ENV_VOLUME > 0 || SQ1_ENV_ADD;
			const bool sq1_length_enable = (!SQ1_LENGTH_ENABLE || (SQ1_LENGTH_ENABLE && SQ1_LENGTH_LOAD));
			SET_APU_STATUS_SQ1(sq1_volume_enable && sq1_length_enable);
			if (length_became_enabled)
			{
				apu_clock_sq1_length();
			}
		}
		else if (SQ1_LENGTH_LOAD > 0 && length_became_enabled && is_first_half_of_frame_clock)
		{
			apu_clock_sq1_length();
		}
	}
	// Square 2
	if (addr == 0xFF15)
	{
		audio_registers->nr2[0] = value;
	}
	else if (addr == 0xFF16)
	{
		audio_registers->nr2[1] = value;
		if (SQ2_LENGTH_LOAD == 0)
		{
			sq2_length_counter = 64;
			SET_SQ2_LENGTH_LOAD(63);
		}
		else
		{
			sq2_length_counter = 64 - (value & 0b00111111);
			SET_SQ2_LENGTH_LOAD(sq2_length_counter);
		}
	}
	else if (addr == 0xFF17)
	{
		audio_registers->nr2[2] = value;
		sq2_env_volume = SQ2_ENV_VOLUME;
		sq2_env_counter = SQ2_ENV_PERIOD > 0 ? SQ2_ENV_PERIOD : 8;
		if (SQ2_ENV_VOLUME == 0)
		{
			SET_APU_STATUS_SQ2(0);
		}
	}
	else if (addr == 0xFF18)
	{
		audio_registers->nr2[3] = value;
	}
	else if (addr == 0xFF19)
	{
		const bool length_enable_prev = SQ2_LENGTH_ENABLE;
		audio_registers->nr2[4] = value;
		const bool length_became_enabled = !length_enable_prev && SQ2_LENGTH_ENABLE;
		const bool is_first_half_of_frame_clock = apu_clock_counter % 2 == 0;
		const bool triggered = SQ2_TRIGGER;
		if (triggered)
		{
			SET_SQ2_TRIGGER(0);
			sq2_freq_counter = 4 * (2048 - SQ2_FREQUENCY);
			sq2_env_volume = SQ2_ENV_VOLUME;
			sq2_env_counter = SQ2_ENV_PERIOD > 0 ? SQ2_ENV_PERIOD : 8;

			if (SQ2_LENGTH_LOAD == 0)
			{
				sq2_length_counter = 64;
				SET_SQ2_LENGTH_LOAD(63);
			}

			const bool sq2_volume_enable = SQ2_ENV_VOLUME > 0 || SQ2_ENV_ADD;
			const bool sq2_length_enable = (!SQ2_LENGTH_ENABLE || (SQ2_LENGTH_ENABLE && SQ2_LENGTH_LOAD));
			SET_APU_STATUS_SQ2(sq2_volume_enable && sq2_length_enable);
			if (length_became_enabled)
			{
				apu_clock_sq2_length();
			}
		}
		else if (SQ2_LENGTH_LOAD > 0 && length_became_enabled && is_first_half_of_frame_clock)
		{
			apu_clock_sq2_length();
		}
	}
	// Wave
	else if (addr == 0xFF1A)
	{
		audio_registers->nr3[0] = value;
		if (WAVE_DAC_POWER == 0)
		{
			SET_APU_STATUS_WAVE(0);
		}
	}
	else if (addr == 0xFF1B)
	{
		audio_registers->nr3[1] = value;
		wave_length_counter = 256 - value;
		if (value == 0)
		{
			SET_WAVE_LENGTH_LOAD(255);
		}
		else
		{
			SET_WAVE_LENGTH_LOAD(wave_length_counter);
		}
	}
	else if (addr == 0xFF1C)
	{
		audio_registers->nr3[2] = value;
	}
	else if (addr == 0xFF1D)
	{
		audio_registers->nr3[3] = value;
	}
	else if (addr == 0xFF1E)
	{
		audio_registers->nr3[4] = value;
		if (WAVE_TRIGGER)
		{
			SET_WAVE_TRIGGER(0);
			if (WAVE_LENGTH_LOAD == 0)
			{
				wave_length_counter = 256;
				SET_WAVE_LENGTH_LOAD(255);
			}
			const bool wave_length_enable = (!WAVE_LENGTH_ENABLE || (WAVE_LENGTH_ENABLE && WAVE_LENGTH_LOAD));
			SET_APU_STATUS_WAVE(WAVE_DAC_POWER&& wave_length_enable);
			wave_freq_counter = 2 * (2048 - WAVE_FREQUENCY);
			wave_position = 1;
		}
	}
	// Noise
	else if (addr == 0xFF1F)
	{
		audio_registers->nr4[0] = value;
	}
	else if (addr == 0xFF20)
	{
		audio_registers->nr4[1] = value;
		if (NOISE_LENGTH_LOAD == 0)
		{
			noise_length_counter = 64;
			SET_NOISE_LENGTH_LOAD(63);
		}
		else
		{
			noise_length_counter = 64 - (value & 0b00111111);
			SET_NOISE_LENGTH_LOAD(noise_length_counter);
		}
	}
	else if (addr == 0xFF21)
	{
		audio_registers->nr4[2] = value;
		noise_env_volume = NOISE_ENV_VOLUME;
		noise_env_counter = NOISE_ENV_PERIOD;
		if (NOISE_ENV_VOLUME == 0)
		{
			SET_APU_STATUS_NOISE(0);
		}
	}
	else if (addr == 0xFF22)
	{
		audio_registers->nr4[3] = value;
	}
	else if (addr == 0xFF23)
	{
		audio_registers->nr4[4] = value;
		if (NOISE_TRIGGER)
		{
			SET_NOISE_TRIGGER(0);
			noise_prng = 0xFFFF;
			noise_freq_counter = noise_divisors[NOISE_DIVISOR_CODE] * (1 << NOISE_CLOCK_SHIFT + 1);
			noise_env_volume = NOISE_ENV_VOLUME;
			if (NOISE_LENGTH_LOAD == 0)
			{
				noise_length_counter = 64;
				SET_NOISE_LENGTH_LOAD(63);
			}
			const bool noise_volume_enable = NOISE_ENV_VOLUME > 0 || NOISE_ENV_ADD;
			const bool noise_length_enable = (!NOISE_LENGTH_ENABLE || (NOISE_LENGTH_ENABLE && NOISE_LENGTH_LOAD));
			SET_APU_STATUS_NOISE(noise_volume_enable && noise_length_enable);
		}
	}

	else if (addr == 0xFF24)
	{
		audio_registers->nr5[0] = value;
	}
	else if (addr == 0xFF25)
	{
		audio_registers->nr5[1] = value;
	}
	else if (addr == 0xFF26)
	{
		const bool prev_power_status = APU_POWER_STATUS;
		audio_registers->nr5[2] = value & 0b10000000;
		const bool new_power_status = APU_POWER_STATUS;
		if (prev_power_status && !new_power_status)
		{
			memset(audio_registers->nr1, 0, sizeof(audio_registers->nr1));
			memset(audio_registers->nr2, 0, sizeof(audio_registers->nr2));
			memset(audio_registers->nr3, 0, sizeof(audio_registers->nr3));
			memset(audio_registers->nr4, 0, sizeof(audio_registers->nr4));
			memset(audio_registers->nr5, 0, sizeof(audio_registers->nr5));
		}
		else if (!prev_power_status && new_power_status)
		{
			audio_registers->nr5[2] = 0b10000000;
		}
	}
}

void tick_noise_lfsr()
{
	const bool bit0 = noise_prng & 0b0000000000000001;
	const bool bit1 = (noise_prng & 0b0000000000000010) >> 1;
	const bool res = bit0 ^ bit1;
	noise_prng = ((noise_prng & ~(1 << 15)) | (res << 15));
	if (NOISE_WIDTH_MODE)
	{
		noise_prng = ((noise_prng & ~(1 << 7)) | (res << 7));
	}
	noise_prng = noise_prng >> 1;
}

float lpf_calculate_alpha(float cutoff_freq, float sample_rate)
{
	const float dt = 1.f / sample_rate;
	const float RC = 1.f / (2.f * 3.14159f * cutoff_freq);
	return dt / (RC + dt);
}

float lpf_process_sample(float input, float alpha, float prev_output)
{
	const float output = prev_output + alpha * (input - prev_output);
	return output;
}

#if CORE_RENDER_APU_DEBUG
void render_debug_output_waves(void* pixels)
{
	const uint16_t POSITION_Y = 256;
	const float sample_view_rate = (float)APU_OUT_SAMPLES_PER_FRAME / VRAM_DEBUG_WIDTH;
	int32_t last_pixel_ly = 0;
	int32_t last_pixel_ry = 0;
	for (uint16_t x = 0; x < VRAM_DEBUG_WIDTH; x++)
	{
		const uint32_t index = x * sample_view_rate;
		const float sampleL = apu_out_samples[index * 2];
		const float sampleR = apu_out_samples[index * 2 + 1];

		const int32_t pixel_L = (sampleL * 64) + POSITION_Y;
		const int32_t pixel_R = (sampleL * 64) + POSITION_Y + 128;
		const uint32_t pixel_address_centerL = (4 * (((POSITION_Y + 48) * VRAM_DEBUG_WIDTH) + x));
		const uint32_t pixel_address_centerR = (4 * (((POSITION_Y + 176) * VRAM_DEBUG_WIDTH) + x));
		const uint32_t pixel_address_L = (4 * ((pixel_L * VRAM_DEBUG_WIDTH) + x));
		const uint32_t pixel_address_R = (4 * ((pixel_R * VRAM_DEBUG_WIDTH) + x));
		uint8_t* base = ((uint8_t*)pixels) + pixel_address_centerL;
		base[0] = 255; base[2] = 127; // Center line, blue
		base = ((uint8_t*)pixels) + pixel_address_centerR;
		base[0] = 255; base[3] = 127; // Center line, red

		base = ((uint8_t*)pixels) + pixel_address_L;
		base[0] = 255; base[2] = 255; // Left channel, blue

		uint16_t y = last_pixel_ly < pixel_L ? last_pixel_ly : pixel_L;
		uint16_t y2 = last_pixel_ly < pixel_L ? pixel_L : last_pixel_ly;
		if (x != 0 && last_pixel_ly != pixel_L)
		{
			for (; y < y2; y++)
			{
				base = ((uint8_t*)pixels) + (4 * ((y * VRAM_DEBUG_WIDTH) + x));
				base[0] = 255; base[2] = 255; // Left channel, blue
			}
		}

		base = ((uint8_t*)pixels) + pixel_address_R;
		base[0] = 255; base[3] = 255; // Right channel, red
		y = last_pixel_ry < pixel_R ? last_pixel_ry : pixel_R;
		y2 = last_pixel_ry < pixel_R ? pixel_R : last_pixel_ry;
		if (x != 0 && last_pixel_ry != pixel_R)
		{
			for (; y < y2; y++)
			{
				base = ((uint8_t*)pixels) + (4 * ((y * VRAM_DEBUG_WIDTH) + x));
				base[0] = 255; base[3] = 255; // Left channel, blue
			}
		}
		last_pixel_ly = pixel_L;
		last_pixel_ry = pixel_R;
	}
}

void render_debug_channel(void* pixels, uint8_t* samples, uint16_t position_y)
{
	const uint8_t CHANNEL_SCALE = 2;
	const float sample_view_rate = (float)APU_SAMPLES_PER_FRAME / VRAM_DEBUG_WIDTH;
	int32_t last_pixel_y = 0;
	for (uint16_t x = 0; x < VRAM_DEBUG_WIDTH; x++)
	{
		const uint32_t index = x * sample_view_rate;
		const uint8_t sample = samples[index];
		const int32_t pixel_y = position_y - (sample * CHANNEL_SCALE);
		const uint32_t pixel_address = (4 * ((pixel_y * VRAM_DEBUG_WIDTH) + x));
		uint8_t* base = ((uint8_t*)pixels) + pixel_address;
		base[0] = 255; base[1] = 192; base[2] = 192; base[3] = 192;

		uint16_t y = last_pixel_y < pixel_y ? last_pixel_y : pixel_y;
		uint16_t y2 = last_pixel_y < pixel_y ? pixel_y : last_pixel_y;
		if (x != 0 && last_pixel_y != pixel_y)
		{
			for (; y < y2; y++)
			{
				base = ((uint8_t*)pixels) + (4 * ((y * VRAM_DEBUG_WIDTH) + x));
				base[0] = 255; base[1] = 192; base[2] = 192; base[3] = 192;
			}
		}
		last_pixel_y = pixel_y;
	}
}

void render_channels_debug_info()
{
	SDL_SetRenderDrawColor(gfx_debug_ctx->renderer, 255, 255, 255, 255);
	SDL_RenderDebugTextFormat(gfx_debug_ctx->renderer, 8, 4, "SQ1 - Status: %s, %s, Sweep P: %s%i, S: %i, Duty: %i",
		APU_STATUS_SQ1 ? "1" : "0",
		!APU_ENABLE_SQ1_LEFT && !APU_ENABLE_SQ1_RIGHT ? "Disabled" :
		APU_ENABLE_SQ1_LEFT && APU_ENABLE_SQ1_RIGHT ? "Left+Right" :
		APU_ENABLE_SQ1_LEFT ? "Left" :
		APU_ENABLE_SQ1_RIGHT ? "Right" : "",
		SQ1_SWEEP_NEGATE ? "-" : "+",
		SQ1_SWEEP_PERIOD,
		SQ1_SWEEP_SHIFT,
		SQ1_DUTY);
	SDL_RenderDebugTextFormat(gfx_debug_ctx->renderer, 8, 14, "Length Load: %i %s, Env Vol: %s%i, Period: %i, Freq: %i",
		SQ1_LENGTH_LOAD,
		SQ1_LENGTH_ENABLE ? "Enabled" : "Disabled",
		SQ1_ENV_ADD ? "+" : "-",
		SQ1_ENV_VOLUME,
		SQ1_ENV_PERIOD,
		SQ1_FREQUENCY);

	SDL_RenderDebugTextFormat(gfx_debug_ctx->renderer, 8, 52, "SQ2 - Status: %s, %s, Duty: %i",
		APU_STATUS_SQ2 ? "1" : "0",
		!APU_ENABLE_SQ2_LEFT && !APU_ENABLE_SQ2_RIGHT ? "Disabled" :
		APU_ENABLE_SQ2_LEFT && APU_ENABLE_SQ2_RIGHT ? "Left+Right" :
		APU_ENABLE_SQ2_LEFT ? "Left" :
		APU_ENABLE_SQ2_RIGHT ? "Right" : "",
		SQ2_DUTY);
	SDL_RenderDebugTextFormat(gfx_debug_ctx->renderer, 8, 62, "Length Load: %i %s, Env Vol: %s%i, Period: %i, Freq: %i",
		SQ2_LENGTH_LOAD,
		SQ2_LENGTH_ENABLE ? "Enabled" : "Disabled",
		SQ2_ENV_ADD ? "+" : "-",
		SQ2_ENV_VOLUME,
		SQ2_ENV_PERIOD,
		SQ2_FREQUENCY);

	SDL_RenderDebugTextFormat(gfx_debug_ctx->renderer, 8, 100, "Wave - Status: %s, %s, DAC: %s",
		APU_STATUS_WAVE ? "1" : "0",
		!APU_ENABLE_WAVE_LEFT && !APU_ENABLE_WAVE_RIGHT ? "Disabled" :
		APU_ENABLE_WAVE_LEFT && APU_ENABLE_WAVE_RIGHT ? "Left+Right" :
		APU_ENABLE_WAVE_LEFT ? "Left" :
		APU_ENABLE_WAVE_RIGHT ? "Right" : "",
		WAVE_DAC_POWER ? "1" : "0");
	SDL_RenderDebugTextFormat(gfx_debug_ctx->renderer, 8, 110, "Length Load: %i %s, Vol Code: %i, Freq: %i",
		WAVE_LENGTH_LOAD,
		WAVE_LENGTH_ENABLE ? "Enabled" : "Disabled",
		WAVE_VOLUME_CODE,
		WAVE_FREQUENCY);

	SDL_RenderDebugTextFormat(gfx_debug_ctx->renderer, 8, 152, "Noise - Status: %s, %s, Divisor: %i, Width Mode: %i",
		APU_STATUS_NOISE ? "1" : "0",
		!APU_ENABLE_NOISE_LEFT && !APU_ENABLE_NOISE_RIGHT ? "Disabled" :
		APU_ENABLE_NOISE_LEFT && APU_ENABLE_NOISE_RIGHT ? "Left+Right" :
		APU_ENABLE_NOISE_LEFT ? "Left" :
		APU_ENABLE_NOISE_RIGHT ? "Right" : "",
		NOISE_DIVISOR_CODE,
		NOISE_WIDTH_MODE);
	SDL_RenderDebugTextFormat(gfx_debug_ctx->renderer, 8, 162, "Length Load: %i %s, Env Vol: %s%i, Period: %i, Clk Shift: %i",
		NOISE_LENGTH_LOAD,
		NOISE_LENGTH_ENABLE ? "Enabled" : "Disabled",
		NOISE_ENV_ADD ? "+" : "-",
		NOISE_ENV_VOLUME,
		NOISE_ENV_PERIOD,
		NOISE_CLOCK_SHIFT);
}

void render_debug_channels(void* pixels)
{
	render_debug_channel(pixels, sq1_sample_buffer, 48);
	render_debug_channel(pixels, sq2_sample_buffer, 96);
	render_debug_channel(pixels, wave_sample_buffer, 144);
	render_debug_channel(pixels, noise_sample_buffer, 192);
	render_channels_debug_info();
}

void apu_render_debug()
{
	void* pixels = NULL;
	int pitch = 0;

	int result = SDL_LockTexture(gfx_debug_ctx->texture, NULL, &pixels, &pitch);
	memset(pixels, 0, 4 * VRAM_DEBUG_WIDTH * VRAM_DEBUG_HEIGHT);

	SDL_SetRenderDrawColor(gfx_debug_ctx->renderer, 32, 32, 32, 255);
	result = SDL_RenderClear(gfx_debug_ctx->renderer);

	render_debug_output_waves(pixels);
	render_debug_channels(pixels);

	SDL_UnlockTexture(gfx_debug_ctx->texture);
	SDL_FRect rect = { 0, 0, VRAM_DEBUG_WIDTH, VRAM_DEBUG_HEIGHT };
	SDL_RenderTexture(gfx_debug_ctx->renderer, gfx_debug_ctx->texture, &rect, &rect);
	SDL_RenderPresent(gfx_debug_ctx->renderer);
}

#endif