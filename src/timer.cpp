#include <timer.h>
#include "pico/stdlib.h"


namespace GameStation
{
    void Timer::start()
    {
        m_start = to_us_since_boot(get_absolute_time());
        m_end = m_start;
        m_tick =  0;
    }

    void Timer::update()
    {
        m_end = to_us_since_boot(get_absolute_time());
        m_delta_time = (static_cast<float>(m_end) - static_cast<float>(m_start)) / 1000000.f;
    }
}
