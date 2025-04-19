#pragma once

#include <cstdint>

namespace GameStation
{

    class Timer
    {
    private:
        uint64_t m_tick, m_start, m_end;
        float m_delta_time;
    public:
        void start();
        void update();

        uint64_t get_tick() { return m_tick; }
        float get_delta_time() { return m_delta_time; }
    };
}