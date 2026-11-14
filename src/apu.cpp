#include "apu.h"
#include "memory_bus.h"
#include "debug_log.h"
#include <SDL3/SDL.h>

const uint32_t APU_SAMPLE_RATE = 48000;
const uint32_t APU_SAMPLES_PER_FRAME = 17556;
const uint32_t APU_OUT_SAMPLES_PER_FRAME = (APU_SAMPLE_RATE / 60);
const float APU_OUT_DIVISOR = (float)APU_OUT_SAMPLES_PER_FRAME / APU_SAMPLES_PER_FRAME;

uint8_t sq1_sample_latest = 0;
uint8_t sq2_sample_latest = 0;
uint32_t apu_out_sample_index = 0;
float apu_out_samples[APU_OUT_SAMPLES_PER_FRAME * 2]; // Stereo output buffer
float apu_out_accumulator = 0.f;

SDL_AudioStream* apu_stream = NULL;

audio_register_set* audio_registers = (audio_register_set*)(memory + 0xFF10);

const uint8_t square_duty_cycles[32] = {
	0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 1, 1,
	0, 1, 1, 1, 1, 1, 1, 0
};

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

uint8_t wave_table[16] = { 0x84, 0x40, 0x43, 0xAA, 0x2D, 0x78, 0x92, 0x3C, 0x60, 0x59, 0xB0, 0x2E, 0xDA };

void apu_init()
{
	SDL_AudioSpec spec;
	spec.channels = 2;
	spec.format = SDL_AUDIO_F32;
	spec.freq = APU_SAMPLE_RATE;
	apu_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
	if (!apu_stream) {
		debug_log("APU failed to create SDL audio stream\n");
		return;
	}
	SDL_ResumeAudioStreamDevice(apu_stream);
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
		if (sq1_freq_counter == 0)
		{
			sq1_freq_counter = 4 * (2048 - SQ1_FREQUENCY);
			sq1_phase = (sq1_phase + 1) % 8;
		}

		sq2_freq_counter -= 1;
		if (sq2_freq_counter == 0)
		{
			sq2_freq_counter = 4 * (2048 - SQ2_FREQUENCY);
			sq2_phase = (sq2_phase + 1) % 8;
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

			apu_out_accumulator += APU_OUT_DIVISOR;
			if (apu_out_accumulator >= 1.f)
			{
				apu_out_accumulator -= 1.f;
				float sampleL = 0.f;
				float sampleR = 0.f;

				const float sq1_sample = TO_FLOAT_SAMPLE(sq1_sample_latest);
				const float sq2_sample = TO_FLOAT_SAMPLE(sq2_sample_latest);

				if (APU_ENABLESQ1_LEFT) sampleL += sq1_sample;
				if (APU_ENABLESQ1_RIGHT) sampleR += sq1_sample;

				if (APU_ENABLESQ2_LEFT) sampleL += sq2_sample;
				if (APU_ENABLESQ2_RIGHT) sampleR += sq2_sample;

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
			audio_registers->nr1[4] &= 0b11111000;          // Clear Frequency MSB bits
			audio_registers->nr1[4] = new_frequency >> 8;   // Frequency MSB write
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
			sq2_env_counter -= 1;
			if (sq2_env_counter == 0)
			{
				sq2_env_counter = SQ2_ENV_PERIOD;
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
		const bool is_first_half_of_frame_clock = (apu_clock_counter % 2 == 0);
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
		const bool is_first_half_of_frame_clock = (apu_clock_counter % 2 == 0);
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
		audio_registers->nr5[2] = value;
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