#pragma once

#include <cstdint>

namespace GameStation
{
    class Renderer
    {
    private:
        static uint8_t s_hn, s_dot_scale;
        static uint16_t s_scrnw, s_scrnh;
        static uint8_t *s_screen_buffer;
    public:
        static void init(const uint16_t scrn_w, const uint16_t scrn_h);
        static void shutdown();
        
        static void draw_pixel(int16_t x, int16_t y);
        static void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
        static void draw_horizontal_line(int16_t x1, int16_t x2, int16_t y);
        static void draw_vertical_line(int16_t y1, int16_t y2, int16_t x);
        static void draw_circle(int16_t center_x, int16_t center_y, uint16_t radius);
        static void draw_filled_circle(int16_t center_x, int16_t center_y, uint16_t radius);
        static void draw_rectangle(int16_t top_left_x, int16_t top_left_y, uint16_t width, uint16_t height);
        static void draw_filled_rectangle(int16_t top_left_x, int16_t top_left_y, uint16_t width, uint16_t height);
        static void draw_character(int16_t top_left_x, int16_t top_left_y, const uint8_t *character);

        static void clear();
        static void show();
    };
}