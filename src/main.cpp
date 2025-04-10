#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "pico/divider.h"
#include <math.h>

#define L3 0
#define L_X 2
#define L_Y 4

#define R3 1
#define R_X 3
#define R_Y 5

#define ANALOG_MAX_VOLTAGE 3.3f
#define ANALOG_PIN 28
#define ANALOG_INPUT 2

#define SPEAKER 6

#define RS 27
#define RW 9
#define E 26
#define CS 14
#define RST 17

#define WIDTH 240
#define HEIGHT 64

#define HN 30
#define DOT_SCALE 8

#define DELAY 5

#define ROWS 8


static const float analog_conversion_factor = 3.3f / (1 << 12);

static const __uint8_t DB_Pins[] = { 10, 21, 11, 20, 12, 19, 13, 18 };
static __uint8_t buffer[HN * HEIGHT];

uint8_t font[36][ROWS] = {
    // '0'
    { 0b01111100,  // Original 0b00111110 invertido => 0b01111100
      0b10000010,  // 0b01000001 -> 0b10000010
      0b10000010,
      0b10000010,
      0b10000010,
      0b10000010,
      0b01111100,
      0b00000000 },
      
    // '1'
    { 0b00001000,  // 0b00010000 -> 0b00001000
      0b00001100,  // 0b00011000 permanece igual
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00111110 }, // 0b01111100 -> 0b00111110
      
    // '2'
    { 0b00111110,  // 0b01111100 -> 0b00111110
      0b01000001,  // 0b10000010 -> 0b01000001
      0b01000000,  // 0b00000010 -> 0b01000000
      0b00100000,  // 0b00000100 -> 0b00100000
      0b00010000,  // 0b00001000 -> 0b00010000
      0b00001000,  // 0b00010000 -> 0b00001000
      0b01111110,  // 0b01111110 permanece igual
      0b00000000 },
      
    // '3'
    { 0b00111110,  // 0b01111100 -> 0b00111110
      0b01000001,  // 0b10000010 -> 0b01000001
      0b01000000,  // 0b00000010 -> 0b01000000
      0b00100000,  // 0b00000100 -> 0b00100000
      0b01000000,  // 0b00000010 -> 0b01000000
      0b01000001,  // 0b10000010 -> 0b01000001
      0b00111110,  // 0b01111100 -> 0b00111110
      0b00000000 },
      
    // '4'
    { 0b00001000,  // 0b00010000 -> 0b00001000
      0b00001100,  // 0b00110000 -> 0b00001100
      0b00001010,  // 0b01010000 -> 0b00001010
      0b00001001,  // 0b10010000 -> 0b00001001
      0b11111111,  // 0b11111111 permanece igual
      0b00001000,  // 0b00010000 -> 0b00001000
      0b00001000,  // 0b00010000 -> 0b00001000
      0b00000000 },
      
    // '5'
    { 0b01111110,  // 0b01111110 permanece igual
      0b00000010,  // 0b01000000 -> 0b00000010
      0b00000010,  // idem
      0b00111110,  // 0b01111100 -> 0b00111110
      0b01000000,  // 0b00000001 -> 0b10000000
      0b01000010,  // 0b01000001 -> 0b10000010
      0b00111110,  // 0b01111100 -> 0b00111110
      0b00000000 },
      
    // '6'
    { 0b00111110,  // 0b01111100 -> 0b00111110
      0b00000010,  // 0b01000000 -> 0b00000010
      0b00000010,  // idem
      0b00111110,  // 0b01111110 permanece
      0b00100010,  // 0b01000001 -> 0b10000010
      0b00100010,  // idem
      0b00111110,  // 0b01111100 -> 0b00111110
      0b00000000 },
      
    // '7'
    { 0b01111100,  // 0b01111110 permanece
      0b01000000,  // 0b01000000 -> 0b00000010
      0b01000000,  // idem
      0b01111000,  // 0b00010000 -> 0b00001000
      0b00001000,  // idem
      0b00001000,  // idem
      0b00001000,  // idem
      0b00000000 },
      
    // '8'
    { 0b01111100,  // 0b01111100 -> 0b00111110
      0b10000010,  // 0b01000001 -> 0b10000010
      0b10000010,  // idem
      0b01111100,  // 0b00111110 invertido de 00111110 = 0b01111100
      0b10000010,  // 0b01000001 -> 0b10000010
      0b10000010,  // idem
      0b01111100,  // 0b01111100 -> 0b00111110
      0b00000000 },
      
    // '9'
    { 0b11111110,  // 0b01111100 -> 0b00111110
      0b10000010,  // 0b01000001 -> 0b10000010
      0b10000010,  // idem
      0b11111110,  // 0b00111110 -> 0b00111100
      0b10000000,  // 0b00000001 -> 0b10000000
      0b10000000,  // 0b01000001 -> 0b10000010
      0b11111100,  // 0b01111100 -> 0b00111110
      0b00000000 },
      
    // 'A'
    { 0b00111100,
      0b01000010,
      0b01000010,
      0b01000010,
      0b01111110,
      0b01000010,
      0b01000010,
      0b00000000 },
      
    // 'B'
    { 0b00111110,  // 0b01111100 -> 0b00111110
      0b01000010,
      0b01000010,
      0b00111110,  // 0b01111100 -> 0b00111110
      0b01000010,
      0b01000010,
      0b00111110,  // 0b01111100 -> 0b00111110
      0b00000000 },
      
    // 'C'
    { 0b00111100,
      0b01000010,
      0b00000010,  // 0b01000000 -> 0b00000010
      0b00000010,  // idem
      0b00000010,  // idem
      0b01000010,
      0b00111100,
      0b00000000 },
      
    // 'D'
    { 0b00011110,  // 0b01111000 -> 0b00011110
      0b00100010,  // 0b01000100 -> 0b00100010
      0b01000010,
      0b01000010,
      0b01000010,
      0b00100010,
      0b00011110,
      0b00000000 },
      
    // 'E'
    { 0b01111110,
      0b00000010,  // 0b01000000 -> 0b00000010
      0b00000010,
      0b00111110,  // 0b01111100 -> 0b00111110
      0b00000010,
      0b00000010,
      0b01111110,
      0b00000000 },
      
    // 'F'
    { 0b01111110,
      0b00000010,
      0b00000010,
      0b00111110,
      0b00000010,
      0b00000010,
      0b00000010,
      0b00000000 },
      
    // 'G'
    { 0b00111100,
      0b01000010,
      0b00000010,
      0b01110010,  // 0b01001110 -> 0b01110010
      0b01000010,
      0b01000010,
      0b00111100,
      0b00000000 },
      
    // 'H'
    { 0b01000010,
      0b01000010,
      0b01111110,
      0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b00000000 },
      
    // 'I'
    { 0b00111100,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00111100,
      0b00000000 },
      
    // 'J'
    { 0b01110000,  // 0b00001110 -> 0b01110000
      0b00100000,  // 0b00000100 -> 0b00100000
      0b00100000,
      0b00100000,
      0b00100010,  // 0b01000100 -> 0b00100010
      0b00100010,
      0b00011100,  // 0b00111000 -> 0b00011100
      0b00000000 },
      
    // 'K'
    { 0b01000010,
      0b00100010,  // 0b01000100 -> 0b00100010
      0b00010010,  // 0b01001000 -> 0b00010010
      0b00001010,  // 0b01010000 -> 0b00001010
      0b00000110,  // 0b01100000 -> 0b00000110
      0b00010010,  // idem
      0b00100010,
      0b00000000 },
      
    // 'L'
    { 0b00000010,  // 0b01000000 -> 0b00000010
      0b00000010,
      0b00000010,
      0b00000010,
      0b00000010,
      0b00000010,
      0b01111110,
      0b00000000 },
      
    // 'M'
    { 0b01000010,
      0b01100110,
      0b01011010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b00000000 },
      
    // 'N'
    { 0b01000010,
      0b01000110,  // 0b01100010 -> 0b01000110
      0b01001010,  // 0b01010010 -> 0b01001010
      0b01010010,  // 0b01001010 -> 0b01010010
      0b01100010,  // 0b01000110 -> 0b01100010
      0b01000010,
      0b01000010,
      0b00000000 },
      
    // 'O'
    { 0b00111100,
      0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b00111100,
      0b00000000 },
      
    // 'P'
    { 0b00111110,  // 0b01111100 -> 0b00111110
      0b01000010,
      0b01000010,
      0b00111110,
      0b00000010,  // 0b01000000 -> 0b00000010
      0b00000010,
      0b00000010,
      0b00000000 },
      
    // 'Q'
    { 0b00111100,
      0b01000010,
      0b01000010,
      0b01000010,
      0b01010010,  // 0b01001010 -> 0b01010010
      0b01100010,  // 0b01000110 -> 0b01100010
      0b10111100,  // 0b00111101 -> 0b10111100
      0b00000000 },
      
    // 'R'
    { 0b00111110,  // 0b01111100 -> 0b00111110
      0b01000010,
      0b01000010,
      0b00111110,
      0b00100010,  // 0b01000100 -> 0b00100010
      0b01000010,
      0b01000010,
      0b00000000 },
      
    // 'S'
    { 0b01111100,  // 0b00111110 reversed -> 0b01111100
      0b00000010,
      0b00000010,
      0b01111100,  // 0b00111110 -> 0b01111100
      0b10000000,  // 0b00000001 -> 0b10000000
      0b10000010,  // 0b01000001 -> 0b10000010
      0b01111100,  // 0b00111110 -> 0b01111100
      0b00000000 },
      
    // 'T'
    { 0b11111110,  // 0b01111111 -> 0b11111110
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00001000,
      0b00000000 },
      
    // 'U'
    { 0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b00111100,
      0b00000000 },
      
    // 'V'
    { 0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b00100100,
      0b00100100,
      0b00011000,
      0b00000000 },
      
    // 'W'
    { 0b01000010,
      0b01000010,
      0b01000010,
      0b01000010,
      0b00101010,  // 0b01010100 -> 0b00101010
      0b00101010,  // idem
      0b00010100,  // 0b00101000 -> 0b00010100
      0b00000000 },
      
    // 'X'
    { 0b01000010,
      0b01000010,
      0b00100100,
      0b00011000,
      0b00011000,
      0b00100100,
      0b01000010,
      0b00000000 },
      
    // 'Y'
    { 0b01000010,
      0b01000010,
      0b00100100,
      0b00011000,
      0b00011000,
      0b00001000,  // 0b00010000 -> 0b00001000
      0b00001000,
      0b00000000 },
      
    // 'Z'
    { 0b01111110,
      0b00100000,  // 0b00000100 -> 0b00100000
      0b00010000,  // 0b00001000 -> 0b00010000
      0b00001000,  // 0b00010000 -> 0b00001000
      0b00000100,  // 0b00100000 -> 0b00000100
      0b00000010,  // 0b01000000 -> 0b00000010
      0b01111110,
      0b00000000 },
};

uint32_t millis()
{
    return to_ms_since_boot(get_absolute_time());
}

uint64_t micro()
{
    return to_us_since_boot(get_absolute_time());
}

void write_byte(__uint8_t byte)
{
    for(__uint32_t i = 0; i < 8; i++)
    {
        int value = (byte & (1 << i)) != 0;
        gpio_put(DB_Pins[i], value);
    }
}

void send_command(__uint8_t command, __uint8_t data)
{
    gpio_put(RW, false);
    gpio_put(RS, true);
    gpio_put(E, true);
    write_byte(command);
    gpio_put(E, false);

    sleep_us(DELAY);
    
    gpio_put(RW, false);
    gpio_put(RS, false);
    gpio_put(E, true);
    write_byte(data);
    gpio_put(E, false);

    sleep_us(DELAY);
}

void clear()
{
    for(__uint32_t i = 0; i < 30*64; i++)
        buffer[i] = 0;
}

void draw_pixel(int x, int y)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;
        
    int byte_index = x / DOT_SCALE;
    int bit_offset = x % DOT_SCALE;

    buffer[y * HN + byte_index] = buffer[y * HN + byte_index] | (1 << bit_offset);
}

void draw_line(int x0, int y0, int x1, int y1) 
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) 
    {
        draw_pixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        
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

void draw_horizontal_line(int x1, int x2, int y) 
{
    if (y >= 0 && y < HEIGHT) 
    {
        if (x1 > x2) 
        {
            int tmp = x1;
            x1 = x2;
            x2 = tmp;
        }

        if (x1 < 0) x1 = 0;
        if (x2 >= WIDTH) x2 = WIDTH - 1;

        for (int x = x1; x <= x2; x++)
            draw_pixel(x, y);
    }
}

void draw_circle(int center_x, int center_y, int radius)
{
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

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

void draw_filled_circle(int center_x, int center_y, int radius) 
{
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

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

void draw_rectangle(int top_left_x, int top_left_y, int width, int height) 
{
    // Desenhar bordas horizontais
    for (int x = top_left_x; x < top_left_x + width; x++) 
    {
        draw_pixel(x, top_left_y); // Borda superior
        draw_pixel(x, top_left_y + height - 1); // Borda inferior
    }
    // Desenhar bordas verticais
    for (int y = top_left_y; y < top_left_y + height; y++) 
    {
        draw_pixel(top_left_x, y); // Borda esquerda
        draw_pixel(top_left_x + width - 1, y); // Borda direita
    }
}

void draw_filled_rectangle(int top_left_x, int top_left_y, int width, int height) 
{
    for (int y = top_left_y; y < top_left_y + height; y++)
        for (int x = top_left_x; x < top_left_x + width; x++)
            draw_pixel(x, y);
}

void draw_character(int top_left_x, int top_left_y, const __uint8_t *character)
{
    if (top_left_x < 0 || top_left_x > WIDTH || top_left_y < 0 || top_left_y > HEIGHT)
        return;
        
    int byte_index = top_left_x / DOT_SCALE;
    int bit_offset = top_left_x % DOT_SCALE;

    for (__uint8_t i = 0; i < ROWS; i++)
    {
        if (top_left_y + i < HEIGHT)
            buffer[(top_left_y + i)* HN + byte_index] = buffer[(top_left_y + i) * HN + byte_index] | (character[i] << bit_offset);
    }

    if (bit_offset > 0 && (byte_index + 1) < HN)
    {
        byte_index += 1;
        for (__uint8_t i = 0; i < ROWS; i++)
        {
            if (top_left_y + i < HEIGHT)
                buffer[(top_left_y + i)* HN + byte_index] = buffer[(top_left_y + i) * HN + byte_index] | (character[i] >> (8 - bit_offset));
        }
    }
}

void show()
{
    send_command(0x0A, 0);
    send_command(0x0B, 0);
    for(__uint32_t i = 0; i < HN*HEIGHT; i++)
        send_command(0x0C, buffer[i]);
}

void init_graphic_mode()
{
    printf("Modo\n");
    send_command(0x00, 0b00100010);

    printf("Hp\n");
    send_command(0x01, 0b00000111);

    printf("HN\n");
    send_command(0x02, 29);

    printf("NX\n");
    send_command(0x03, 63);

    printf("CURSOR\n");
    send_command(0x04, 0);

    printf("LOWER DISPLAY\n");
    send_command(0x08, 0);

    printf("UPPER DISPLAY\n");
    send_command(0x09, 0);

    printf("LOWER CURSOR\n");
    send_command(0x0A, 0);

    printf("UPPER CURSOR\n");
    send_command(0x0B, 0);
}

float get_analog_axis(uint32_t axis)
{
    gpio_put(axis, true);
    float voltage = adc_read() * analog_conversion_factor;
    float normalized = (voltage / ANALOG_MAX_VOLTAGE) * 2.f - 1.f;
    gpio_put(axis, false);
    
    return normalized;
}

#define SAMPLES 100
// Tabela com valores da senoide de 0 a 100%
uint8_t sine_table[SAMPLES];

void init_sine_table() {
    for (int i = 0; i < SAMPLES; i++) {
        float radians = 2.0f * M_PI * ((float)i / (float)SAMPLES);
        // De -1..1 para 0..100%
        sine_table[i] = (uint8_t)((sinf(radians) + 1.0f) * 50.0f);
    }
}

int32_t pwm_set_freq_duty(uint32_t slice_num, uint32_t chan, uint32_t freq, 
                          int duty_cycle)
{

    uint8_t clk_divider = 0;
    uint32_t wrap = 0;
    uint32_t clock_div = 0;
    uint32_t clock = 125000000;

    if(freq < 8 && freq > clock)
       /* This is the frequency range of generating a PWM 
       in RP2040 at 125MHz */
        return -1;

    for(clk_divider = 1; clk_divider < UINT8_MAX; clk_divider++)
    {
        /* Find clock_division to fit current frequency */
        clock_div = div_u32u32( clock, clk_divider );
        wrap = div_u32u32(clock_div, freq);
        if (div_u32u32 (clock_div, UINT16_MAX) <= freq && wrap <= UINT16_MAX)
        {
            break;
        }
    }
    if(clk_divider < UINT8_MAX)
    {
        /* Only considering whole number division */
        pwm_set_clkdiv_int_frac(slice_num, clk_divider, 0);
        pwm_set_wrap(slice_num, (uint16_t) wrap);
        pwm_set_chan_level(slice_num, chan, 
                          (uint16_t) div_u32u32((((uint16_t)(duty_cycle == 100? 
                          (wrap + 1) : wrap)) * duty_cycle), 100));
    }
    else
        return -2;

    return 1;
}

int main() 
{
    stdio_init_all(); // Inicializa a comunicação USB
    init_sine_table();

    sleep_ms(5000);

    gpio_set_function(SPEAKER, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(SPEAKER);
    uint channel = pwm_gpio_to_channel(SPEAKER);
    float div = 1.0f; // divisor do clock
    pwm_set_clkdiv(slice, div);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 4.f);
    pwm_init(slice, &cfg, true);
    
    adc_init();
    adc_gpio_init(ANALOG_PIN);
    adc_select_input(ANALOG_INPUT);

    gpio_init(L3);
    gpio_set_dir(L3, GPIO_IN);
    gpio_pull_up(L3);

    gpio_init(L_X);
    gpio_set_dir(L_X, GPIO_OUT);
    gpio_init(L_Y);
    gpio_set_dir(L_Y, GPIO_OUT);

    gpio_init(R3);
    gpio_set_dir(R3, GPIO_IN);
    gpio_pull_up(R3);
    
    gpio_init(R_X);
    gpio_set_dir(R_X, GPIO_OUT);
    gpio_init(R_Y);
    gpio_set_dir(R_Y, GPIO_OUT);
    
    gpio_init(RS);
    gpio_set_dir(RS, GPIO_OUT);

    gpio_init(RW);
    gpio_set_dir(RW, GPIO_OUT);

    gpio_init(E);
    gpio_set_dir(E, GPIO_OUT);

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);

    gpio_init(RST);
    gpio_set_dir(RST, GPIO_OUT);

    for (__uint8_t i = 0; i < 8; i++)
    {
        gpio_init(DB_Pins[i]);
        gpio_set_dir(DB_Pins[i], GPIO_OUT);
    }

    gpio_put(RST, false);

    sleep_ms(1000);

    gpio_put(RST, true);

    sleep_ms(1000);

    gpio_put(CS, false);

    init_graphic_mode();

    int increment = 0;
    float freq = 60;
    typedef struct _ball
    {
        int x, y, vel_x, vel_y, radius;
    } ball_t;

    ball_t ball_1 = {WIDTH / 4, HEIGHT / 2, 1, 1, 5}, ball_2 {3 * WIDTH / 4, HEIGHT / 2, -1, -1, 5};

    float p_x, p_y;

    __uint32_t start = millis();
    uint64_t start_us = micro();
    float accum = 0;
    uint64_t wave_accum = 0;
    uint32_t sample_index = 0;
    bool is_sined = true, r3_pressed = false;
    while (true) 
    {
        __uint32_t end = millis();
        float delta_time = (end - start) / 1000.f;
        start = end;

        uint64_t end_us = micro();
        uint64_t delta_time_us = (end_us - start_us);
        start_us = end_us;
        uint64_t delay_us = 1e6 / (freq * SAMPLES);

        if (is_sined)
        {
            wave_accum  += delta_time_us;

            while (wave_accum >= delay_us)
            {
                wave_accum -= delay_us;
                sample_index = (sample_index + 1) % SAMPLES;
            }
            pwm_set_freq_duty(slice, channel, freq, sine_table[sample_index]);
        }
        else
        {
            wave_accum = 0.f;
            pwm_set_freq_duty(slice, channel, freq, 50);
        }

        /*

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

        printf("L_X: %.2f -", get_analog_axis(L_X));
        printf("L_Y: %.2f -", get_analog_axis(L_Y));
        printf("R_X: %.2f -", get_analog_axis(R_X));
        printf("R_Y: %.2f\n", get_analog_axis(R_Y));
        printf("FREQ: %u\n", (uint16_t)freq);
        printf("IS_SINED: %d\n", is_sined);
        p_x += delta_time * get_analog_axis(L_X) * 10;
        p_y -= delta_time * get_analog_axis(L_Y) * 10;

        freq += delta_time * get_analog_axis(R_Y) * 1000;
        if (freq < 0) freq = 0;
        else if (freq >= 65535) freq = 65535;

        if (p_x < 0) p_x = 0;
        else if (p_x >= WIDTH) p_x = WIDTH - 1;

        if (p_y < 0) p_y = 0;
        else if (p_y >= HEIGHT) p_y = HEIGHT - 1;

        clear();
        
        draw_circle(ball_1.x, ball_1.y, ball_1.radius);
        draw_filled_circle(ball_2.x, ball_2.y, ball_2.radius);

        int x = 0, y = 0;
        for (__uint8_t i = 0; i < 36; i++)
        {
            draw_character(x + increment, y + increment, font[i]);
            x += 8;

            if (x >= WIDTH)
            {
                x = 0;
                y += 8;
            }
        }
            
        if (!gpio_get(L3))
        {
            draw_character(WIDTH / 4, HEIGHT / 2 - 4, font[21]);
            draw_character(WIDTH / 4 + 8, HEIGHT / 2 - 4, font[3]);
        }

        if (!gpio_get(R3))
        {
            draw_character(WIDTH * 0.75f, HEIGHT / 2 - 4, font['R' - 'A' + 10]);
            draw_character(WIDTH * 0.75f + 8, HEIGHT / 2 - 4, font[3]);
            
            if (!r3_pressed)
                is_sined = !is_sined;

            r3_pressed = true;
        }
        else
            r3_pressed = false;

        draw_filled_circle(p_x, p_y, 2);
        //show();

        accum += delta_time;
        if (accum > 1.0f)
        {
            increment = (increment + 1) % 8;
            accum -= 1.0f;
        }
        */
    }

    return 0;
}