#include <stdio.h>
#include "pico/stdlib.h"

#include <math.h>

#include <lcd.h>
#include <renderer.h>
#include <input.h>
#include <fonts.h>
#include <timer.h>
#include <audio_manager.h>

#define WIDTH 240
#define HEIGHT 64

int main() 
{
    stdio_init_all(); // Inicializa a comunicação USB

    sleep_ms(5000);

    // RW, RS, EN, RST, CS, { D0, D1, D2, D3, D4, D5, D6, D7 }
    GameStation::LCDPinsConfiguration config(9, 27, 26, 17, 14, { 10, 21, 11, 20, 12, 19, 13, 18 });
    GameStation::LCD::init(WIDTH, HEIGHT, config);

    GameStation::Renderer::init(WIDTH, HEIGHT);

    // ANALOG_PIN, ANALOG_INPUT, L3, LX, LY, R3, RX, RY
    GameStation::Input::init(28, 2, 0, 2, 4, 1, 3, 5);

    GameStation::AudioManager::init(6);

    int increment = 0;
    float freq = 60;
    typedef struct _ball
    {
        int x, y, vel_x, vel_y, radius;
    } ball_t;

    ball_t ball_1 = {WIDTH / 4, HEIGHT / 2, 1, 1, 5}, ball_2 {3 * WIDTH / 4, HEIGHT / 2, -1, -1, 5};

    float p_x, p_y;



    float accum = 0;

    GameStation::Timer timer;
    timer.start();
    while (true) 
    {
        timer.update();
        GameStation::Input::update();

        ball_1.x += ball_1.vel_x;
        ball_1.y += ball_1.vel_y;

        ball_2.x += ball_2.vel_x;
        ball_2.y += ball_2.vel_y;

        if (ball_1.x + ball_1.radius > WIDTH - 1)
        {
            ball_1.x = WIDTH - 1 - ball_1.radius;
            ball_1.vel_x *= -1;
        }
        else if (ball_1.x - ball_1.radius < 0)
        {
            ball_1.x = ball_1.radius;
            ball_1.vel_x *= -1;
        }

        if (ball_1.y + ball_1.radius > HEIGHT - 1)
        {
            ball_1.y = HEIGHT - 1 - ball_1.radius;
            ball_1.vel_y *= -1;
        }
        else if (ball_1.y - ball_1.radius < 0)
        {
            ball_1.y = ball_1.radius;
            ball_1.vel_y *= -1;
        }

        if (ball_2.x + ball_2.radius > WIDTH - 1)
        {
            ball_2.x = WIDTH - 1 - ball_2.radius;
            ball_2.vel_x *= -1;
        }
        else if (ball_2.x - ball_2.radius < 0)
        {
            ball_2.x = ball_2.radius;
            ball_2.vel_x *= -1;
        }

        if (ball_2.y + ball_2.radius > HEIGHT - 1)
        {
            ball_2.y = HEIGHT - 1 - ball_2.radius;
            ball_2.vel_y *= -1;
        }
        else if (ball_2.y - ball_2.radius < 0)
        {
            ball_2.y = ball_2.radius;
            ball_2.vel_y *= -1;
        }

        int delta_x = ball_1.x - ball_2.x;
        int delta_y = ball_1.y - ball_2.y;
        int d = sqrt(delta_x * delta_x + delta_y * delta_y);

        if (d <= ball_1.radius + ball_2.radius)
        {
            ball_1.x += delta_x / 2;
            ball_1.y += delta_y / 2;

            ball_2.x -= delta_x / 2;
            ball_2.y -= delta_y / 2;

            int tmp = ball_1.vel_x;
            ball_1.vel_x = ball_2.vel_x;
            ball_2.vel_x = tmp;

            tmp = ball_1.vel_y;
            ball_1.vel_y = ball_2.vel_y;
            ball_2.vel_y = tmp;
        }

        GameStation::InputVec2f dir = GameStation::Input::get_analog(GameStation::AnalogSide::Left);
        p_x += timer.get_delta_time() * dir.x * 10;
        p_y -= timer.get_delta_time() * dir.y * 10;

        if (p_x < 0) p_x = 0;
        else if (p_x >= WIDTH) p_x = WIDTH - 1;

        if (p_y < 0) p_y = 0;
        else if (p_y >= HEIGHT) p_y = HEIGHT - 1;

        GameStation::Renderer::clear();

        GameStation::Renderer::draw_circle(ball_1.x, ball_1.y, ball_1.radius);
        GameStation::Renderer::draw_filled_circle(ball_2.x, ball_2.y, ball_2.radius);

        int x = 0, y = 0;
        for (__uint8_t i = 0; i < 36; i++)
        {
            GameStation::Renderer::draw_character(x + increment, y + increment, default_font[i]);
            x += 8;

            if (x >= WIDTH)
            {
                x = 0;
                y += 8;
            }
        }

        if (GameStation::Input::get_switch_down(GameStation::AnalogSide::Left))
        {
            GameStation::Renderer::draw_character(WIDTH / 4, HEIGHT / 2 - 4, default_font[21]);
            GameStation::Renderer::draw_character(WIDTH / 4 + 8, HEIGHT / 2 - 4, default_font[3]);
        }

        if (GameStation::Input::get_switch_down(GameStation::AnalogSide::Right))
        {
            GameStation::Renderer::draw_character(WIDTH * 0.75f, HEIGHT / 2 - 4, default_font['R' - 'A' + 10]);
            GameStation::Renderer::draw_character(WIDTH * 0.75f + 8, HEIGHT / 2 - 4, default_font[3]);
        }

        GameStation::Renderer::draw_filled_circle(p_x, p_y, 2);

        GameStation::Renderer::show();

        accum += timer.get_delta_time();
        if (accum > 1.0f)
        {
            increment = (increment + 1) % 8;
            accum -= 1.0f;
        }
    }

    GameStation::AudioManager::shutdown();
    GameStation::Renderer::shutdown();

    return 0;
}