#include <audio_manager.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/divider.h"
#include "pico/multicore.h"
#include <string.h>

#include <timer.h>

#define SAMPLE_RATE 44100
#define PWM_TOP 255

namespace GameStation
{
    uint8_t AudioManager::s_speaker, AudioManager::s_current_buffer;
    volatile bool AudioManager::s_running, AudioManager::s_core1_in_use;
    bool AudioManager::s_buffer_read, AudioManager::s_copy_allowed;
    uint32_t AudioManager::s_slice, AudioManager::s_channel;
    std::array<std::array<uint16_t, BLOCK_SAMPLES>, 2> AudioManager::s_sound_buffer;
    spin_lock_t *AudioManager::s_lock;

    void AudioManager::init(uint8_t speaker)
    {
        s_speaker = speaker;
        s_running = true;
        s_current_buffer = 0;
        s_buffer_read = s_copy_allowed = false;

        // Obtendo o primeiro spin lock não reservado e o tornando exclusivo
        s_lock = spin_lock_init(PICO_SPINLOCK_ID_STRIPED_FIRST);
        spin_lock_claim(PICO_SPINLOCK_ID_STRIPED_FIRST);

        // Registrando e configurando pwm para o speaker
        gpio_set_function(speaker, GPIO_FUNC_PWM);
        s_slice = pwm_gpio_to_slice_num(speaker);
        s_channel = pwm_gpio_to_channel(speaker);
        pwm_set_wrap(s_slice, PWM_TOP);
        pwm_set_chan_level(s_slice, s_channel, 0);
        pwm_set_enabled(s_slice, true);

        // Começando paralelização
        s_core1_in_use = true;
        multicore_launch_core1(&AudioManager::audio_loop);
    }

    void AudioManager::shutdown()
    {
        s_running = false;

        while (s_core1_in_use)
        {
            __wfe(); // Espera até que o core 1 tenha finalizado
        }

        // Com o core 1 finalizado podemos dar unclaim com garantia
        // que o spin lock não está travado
        spin_lock_unclaim(PICO_SPINLOCK_ID_STRIPED_FIRST);
    }

    void AudioManager::begin()
    {
        uint32_t saved = spin_lock_blocking(s_lock);
        s_copy_allowed = !s_buffer_read;
        spin_unlock(s_lock, saved);

        if (s_copy_allowed)
            memset(s_sound_buffer[(s_current_buffer + 1) % 2].data(), 0, BLOCK_SAMPLES * sizeof(uint16_t));
    }

    void AudioManager::send_buffer(const uint16_t *buffer, uint32_t n)
    {
        if (!s_copy_allowed || n > BLOCK_SAMPLES) return;

        uint8_t write_buffer = s_current_buffer ^ 1;

        for (uint16_t i = 0; i < n; i++)
        {
            uint16_t sum = s_sound_buffer[write_buffer][i] + buffer[i];

            if (sum < buffer[i])
                sum = UINT16_MAX;
            
            s_sound_buffer[write_buffer][i] = sum;
        }
    }

    void AudioManager::end()
    {
        if (s_copy_allowed)
        {
            uint32_t saved = spin_lock_blocking(s_lock);
            s_buffer_read = true;
            spin_unlock(s_lock, saved);
            
            s_copy_allowed = false;
        }
    }

    void AudioManager::audio_loop()
    {
        float accum = 0.f;
        float period = 1.f / SAMPLE_RATE;
        uint32_t current_index = 0;
        
        Timer timer;
        timer.start();
        while (s_running)
        {
            timer.update();
            accum += timer.get_delta_time();

            pwm_set_chan_level(s_slice, s_channel, s_sound_buffer[s_current_buffer][current_index]);

            while (accum >= period)
            {
                current_index = (current_index + 1) % BLOCK_SAMPLES;
                accum -= period;
                
                if (current_index == 0)
                {
                    // Obs: Se não tiver interrupções no processador 1 então pode mudar para unsafe
                    // Mas isso deixemos para finalização do projeto
                    uint32_t saved = spin_lock_blocking(s_lock);

                    if (s_buffer_read)
                    {
                        s_current_buffer ^= 1;
                        s_buffer_read = false;
                    }

                    spin_unlock(s_lock, saved);
                }
            }
        }

        // Assinalando q o core1 já foi finalizado
        s_core1_in_use = false;
    }
}