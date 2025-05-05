#include <renderer.h>
#include <cmath>
#include <string.h>
#include <lcd.h>
#include <defines.h>

#define MAX(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) ((b)>(a)?(a):(b))

namespace GameStation
{
    uint8_t Renderer::s_hn, Renderer::s_dot_scale;
    uint16_t Renderer::s_scrnw, Renderer::s_scrnh;
    uint8_t* Renderer::s_screen_buffer = nullptr;

    void Renderer::init(const uint16_t scrn_w, const uint16_t scrn_h)
    {
        s_scrnw = scrn_w;
        s_scrnh = scrn_h;
        s_hn = LCD::get_hn();
        s_dot_scale = scrn_w / s_hn;

        s_screen_buffer = new uint8_t[scrn_h * s_hn];
    }

    void Renderer::shutdown()
    {
        delete[] s_screen_buffer;
    }

    void Renderer::draw_pixel(int16_t x, int16_t y)
    {
        if (x < 0 || x >= s_scrnw || y < 0 || y >= s_scrnh)
            return;
        
        uint16_t byte_index = x / s_dot_scale;
        uint16_t bit_offset = x % s_dot_scale;

        s_screen_buffer[y * s_hn + byte_index] = s_screen_buffer[y * s_hn + byte_index] | (1 << bit_offset);
    }

    void Renderer::draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
    {
        uint16_t dx = abs(x1 - x0);
        uint16_t dy = abs(y1 - y0);
        int16_t sx = (x0 < x1) ? 1 : -1;
        int16_t sy = (y0 < y1) ? 1 : -1;
        int16_t err = dx - dy;

        while (true) 
        {
            draw_pixel(x0, y0);
            if (x0 == x1 && y0 == y1) break;
            int16_t e2 = 2 * err;
            
            if (e2 > -dy) 
            {
                err -= dy;
                x0 += sx;
            }
            
            if (e2 < dx) 
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    void Renderer::draw_horizontal_line(int16_t x1, int16_t x2, int16_t y)
    {
        if (y >= 0 && y < s_scrnh) 
        {
            if (x1 > x2) 
            {
                int16_t tmp = x1;
                x1 = x2;
                x2 = tmp;
            }

            if (x1 < 0) x1 = 0;
            if (x2 >= s_scrnw) x2 = s_scrnw - 1;

            int16_t row = y * s_hn;
            for (int16_t x = x1; x <= x2; x++)
            {
                uint16_t byte_index = x / s_dot_scale;
                uint16_t bit_offset = x % s_dot_scale;

                s_screen_buffer[row + byte_index] = s_screen_buffer[row + byte_index] | (1 << bit_offset);
            }
        }
    }

    void Renderer::draw_vertical_line(int16_t y1, int16_t y2, int16_t x)
    {
        if (x >= 0 && x < s_scrnw) 
        {
            if (y1 > y2) 
            {
                int16_t tmp = y1;
                y1 = y2;
                y2 = tmp;
            }

            if (y1 < 0) y1 = 0;
            if (y2 >= s_scrnh) y2 = s_scrnh - 1;

            uint16_t byte_index = x / s_dot_scale;
            uint16_t bit_offset = x % s_dot_scale;

            for (int16_t y = y1; y <= y2; y++)
                s_screen_buffer[y * s_hn + byte_index] = s_screen_buffer[y * s_hn + byte_index] | (1 << bit_offset);
        }
    }

    void Renderer::draw_circle(int16_t center_x, int16_t center_y, uint16_t radius)
    {
        int16_t x = 0;
        int16_t y = radius;
        int16_t d = 3 - 2 * radius;

        while (y >= x) 
        {
            // Desenha os oito octantes da circunferência
            draw_pixel(center_x + x, center_y + y);
            draw_pixel(center_x - x, center_y + y);
            draw_pixel(center_x + x, center_y - y);
            draw_pixel(center_x - x, center_y - y);
            draw_pixel(center_x + y, center_y + x);
            draw_pixel(center_x - y, center_y + x);
            draw_pixel(center_x + y, center_y - x);
            draw_pixel(center_x - y, center_y - x);

            x++;
            if (d > 0) 
            {
                y--;
                d = d + 4 * (x - y) + 10;
            } 
            else
                d = d + 4 * x + 6;
        }
    }

    void Renderer::draw_filled_circle(int16_t center_x, int16_t center_y, uint16_t radius)
    {
        int16_t x = 0;
        int16_t y = radius;
        int16_t d = 3 - 2 * radius;

        while (y >= x) 
        {
            // Desenha linhas horizontais entre os pontos da circunferência
            draw_horizontal_line(center_x - x, center_x + x, center_y + y);
            draw_horizontal_line(center_x - x, center_x + x, center_y - y);
            draw_horizontal_line(center_x - y, center_x + y, center_y + x);
            draw_horizontal_line(center_x - y, center_x + y, center_y - x);

            x++;
            if (d > 0) 
            {
                y--;
                d = d + 4 * (x - y) + 10;
            } 
            else
                d = d + 4 * x + 6;
        }
    }

    void Renderer::draw_rectangle(int16_t top_left_x, int16_t top_left_y, uint16_t width, uint16_t height)
    {
        // Desenhar bordas horizontais
        draw_horizontal_line(top_left_x, top_left_x + width, top_left_y);
        draw_horizontal_line(top_left_x, top_left_x + width, top_left_y + height - 1);

        // Desenhar bordas verticais
        draw_vertical_line(top_left_y, top_left_y + height, top_left_x);
        draw_vertical_line(top_left_y, top_left_y + height, top_left_x + width - 1);
    }

    void Renderer::draw_filled_rectangle(int16_t top_left_x, int16_t top_left_y, uint16_t width, uint16_t height)
    {
        if (top_left_y < 0) top_left_y = 0;
        if (top_left_x < 0) top_left_x = 0;

        int16_t y2 = MIN(s_scrnh, top_left_y + height);
        int16_t x2 = MIN(s_scrnw, top_left_x + width);

        for (int16_t y = top_left_y; y < y2; y++)
        {
            int16_t row = y * s_hn;

            for (int16_t x = top_left_x; x < x2; x++)
            {
                uint16_t byte_index = x / s_dot_scale;
                uint16_t bit_offset = x % s_dot_scale;
                
                s_screen_buffer[row + byte_index] = s_screen_buffer[row + byte_index] | (1 << bit_offset);
            }
        }
    }

    void Renderer::draw_character(int16_t top_left_x, int16_t top_left_y, const uint8_t *character)
    {
        int16_t x0 = MAX(0, top_left_x);

        int16_t y0 = MAX(0, top_left_y);
        int16_t y1 = MIN(s_scrnh - 1, top_left_y + SPRITE_ROWS);
            
        int16_t byte_index = x0 / s_dot_scale;
        int16_t bit_offset = x0 % s_dot_scale;

        // Se uma parte do sprite estiver para fora da tela do lado esquerdo
        // Desenha apenas a parte da direita
        if (top_left_x < 0)
        {
            for (uint16_t y = y0, i = y0 - top_left_y; y < y1; y++, i++)
                s_screen_buffer[y * s_hn + byte_index] = s_screen_buffer[y * s_hn + byte_index] | (character[i] >> (8 - bit_offset));
        }
        else // Caso contrário desenha a parte esquerda e da direita do sprite
        {
            for (uint16_t y = y0, i = y0 - top_left_y; y < y1; y++, i++)
                s_screen_buffer[y * s_hn + byte_index] = s_screen_buffer[y * s_hn + byte_index] | (character[i] << bit_offset);

            // Se o sprite tiver um offset maior que 1 ent devemos desenhar o lado direito
            // Dele no próximo byte, porém isso apenas se existir um próximo byte a direita
            // Caso contrário a parte direita estará fora da tela e por isso não desenhamos
            if (bit_offset > 0 && byte_index < s_hn - 1)
            {
                byte_index++;
                for (uint16_t y = y0, i = y0 - top_left_y; y < y1; y++, i++)
                    s_screen_buffer[y * s_hn + byte_index] = s_screen_buffer[y * s_hn + byte_index] | (character[i] >> (8 - bit_offset));
            }
        }
    }

    void Renderer::clear()
    {
        memset(s_screen_buffer, 0, s_hn * s_scrnh);
    }

    void Renderer::show()
    {
        LCD::write_screen_buffer(s_screen_buffer);
    }
}