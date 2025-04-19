#pragma once

#include <cstdint>

namespace GameStation
{
    enum class AnalogAxis
    {
        Left_X,
        Left_Y,
        Right_X,
        Right_Y
    };

    enum class AnalogSide
    {
        Left,
        Right
    };

    struct InputVec2f
    {
        float x, y;
    };

    class Input
    {
    private:
        static uint8_t s_left_sw, s_left_x, s_left_y, s_right_x, s_right_y, s_right_sw;
        static bool s_last_left_sw_state, s_left_sw_state, s_last_right_sw_state, s_right_sw_state;
    public:
        static void init(uint8_t adc_pin, uint8_t adc_input, uint8_t left_sw, uint8_t left_x, uint8_t left_y, uint8_t right_sw, uint8_t right_x, uint8_t right_y);

        static void update();

        static float get_analog_axis(AnalogAxis axis);
        static InputVec2f get_analog(AnalogSide side);

        static bool get_switch_down(AnalogSide side);
        static bool get_switch_pressed(AnalogSide side);
        static bool get_switch_release(AnalogSide side);
    };
}