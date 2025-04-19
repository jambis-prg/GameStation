#pragma once
#include <stdint.h>
#include <array>
#include "hardware/sync.h"

#define BLOCK_SAMPLES 1024

namespace GameStation
{
    class AudioManager
    {
    private:
        static uint8_t s_speaker, s_current_buffer;
        static volatile bool s_running, s_core1_in_use;
        static bool s_buffer_read, s_copy_allowed;
        static uint32_t s_slice, s_channel;
        static std::array<std::array<uint16_t, BLOCK_SAMPLES>, 2> s_sound_buffer;
        static spin_lock_t *s_lock;
        
        static void audio_loop();
    public:
        static void init(uint8_t speaker);
        static void shutdown();

        static void begin();
        static void send_buffer(const uint16_t *buffer, uint32_t n = BLOCK_SAMPLES);
        static void end();
    };
}