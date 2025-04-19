#include <input.h>
#include "hardware/adc.h"

#define ANALOG_MAX_VOLTAGE 3.3f

namespace GameStation
{
    uint8_t Input::s_left_sw, Input::s_left_x, Input::s_left_y, Input::s_right_x, Input::s_right_y, Input::s_right_sw;
    bool Input::s_last_left_sw_state, Input::s_left_sw_state, Input::s_last_right_sw_state, Input::s_right_sw_state;

    static constexpr float analog_conversion_factor = 3.3f / (1 << 12);

    void Input::init(uint8_t adc_pin, uint8_t adc_input, uint8_t left_sw, uint8_t left_x, uint8_t left_y, uint8_t right_sw, uint8_t right_x, uint8_t right_y)
    {
        s_left_sw = left_sw;
        s_left_x = left_x;
        s_left_y = left_y;

        s_right_sw = right_sw;
        s_right_x = right_x;
        s_right_y = right_y;

        s_last_left_sw_state = s_left_sw_state = false;
        s_last_right_sw_state = s_right_sw_state = false;

        adc_init();
        adc_gpio_init(adc_pin);
        adc_select_input(adc_input);

        gpio_init(left_sw);
        gpio_set_dir(left_sw, GPIO_IN);
        gpio_pull_up(left_sw);

        gpio_init(left_x);
        gpio_set_dir(left_x, GPIO_OUT);
        gpio_init(left_y);
        gpio_set_dir(left_y, GPIO_OUT);

        gpio_init(right_sw);
        gpio_set_dir(right_sw, GPIO_IN);
        gpio_pull_up(right_sw);
        
        gpio_init(right_x);
        gpio_set_dir(right_x, GPIO_OUT);
        gpio_init(right_y);
        gpio_set_dir(right_y, GPIO_OUT);
    }

    void Input::update()
    {
        s_last_left_sw_state = s_left_sw_state;
        s_left_sw_state = gpio_get(s_left_sw);

        s_last_right_sw_state = s_right_sw_state;
        s_right_sw_state = gpio_get(s_right_sw);
    }

    float Input::get_analog_axis(AnalogAxis axis)
    {
        uint8_t pin;

        switch (axis)
        {
        // L3
        case AnalogAxis::Left_X:
            pin = s_left_x;
            break;
        case AnalogAxis::Left_Y:
            pin = s_left_y;
            break;

        // R3 
        case AnalogAxis::Right_X:
            pin = s_right_x;
            break;
        case AnalogAxis::Right_Y:
            pin = s_right_y;
            break;
        default:
            return 0.f;
        }

        // Selecionando eixo do analógico
        gpio_put(pin, true);

        float voltage = adc_read() * analog_conversion_factor;
        float normalized = (voltage / ANALOG_MAX_VOLTAGE) * 2.f - 1.f;
        
        gpio_put(pin, false);
        
        return normalized;
    }

    InputVec2f Input::get_analog(AnalogSide side)
    {
        uint8_t pin_x, pin_y;

        switch (side)
        {
        // L3
        case AnalogSide::Left:
            pin_x = s_left_x;
            pin_y = s_left_y;
            break;
        // R3
        case AnalogSide::Right:
            pin_x = s_right_x;
            pin_y = s_right_y;
            break;
        default:
            return { 0.f, 0.f };
        }

        InputVec2f result;

        // Selecionando eixo x do analógico
        gpio_put(pin_x, true);

        float voltage = adc_read() * analog_conversion_factor;
        float normalized = (voltage / ANALOG_MAX_VOLTAGE) * 2.f - 1.f;
        result.x = normalized;
        
        gpio_put(pin_x, false);

        // Selecionando eixo y do analógico
        gpio_put(pin_y, true);

        voltage = adc_read() * analog_conversion_factor;
        normalized = (voltage / ANALOG_MAX_VOLTAGE) * 2.f - 1.f;
        result.y = normalized;

        gpio_put(pin_y, false);
        
        return result;
    }

    bool Input::get_switch_down(AnalogSide side)
    {
        switch (side)
        {
        // L3
        case AnalogSide::Left:
            return s_left_sw_state;
        // R3
        case AnalogSide::Right:
            return s_right_sw_state;
        default:
            return false;
        }
    }

    bool Input::get_switch_pressed(AnalogSide side)
    {
        switch (side)
        {
        // L3
        case AnalogSide::Left:
            return !s_last_left_sw_state && s_left_sw_state;
        // R3
        case AnalogSide::Right:
            return !s_last_right_sw_state && s_right_sw_state;
        default:
            return false;
        }
    }
    
    bool Input::get_switch_release(AnalogSide side)
    {
        switch (side)
        {
        // L3
        case AnalogSide::Left:
            return s_last_left_sw_state && !s_left_sw_state;
        // R3
        case AnalogSide::Right:
            return s_last_right_sw_state && !s_right_sw_state;
        default:
            return false;
        }
    }
}