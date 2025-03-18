#include <stdio.h>
#include "pico/stdlib.h"


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

static const __uint8_t DB_Pins[] = { 10, 21, 11, 20, 12, 19, 13, 18 };
static __uint8_t buffer[HN * HEIGHT];

uint32_t millis()
{
    return to_ms_since_boot(get_absolute_time());
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
    int byte_index = x / DOT_SCALE;
    int bit_offset = x % DOT_SCALE;

    buffer[y * HN + byte_index] = buffer[y * HN + byte_index] | (1 << bit_offset);
}

void show()
{
    send_command(0x0A, 0);
    send_command(0x0B, 0);
    for(__uint32_t i = 0; i < 30*64; i++)
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

int main() {
    stdio_init_all(); // Inicializa a comunicação USB

    sleep_ms(5000);

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
    while (true) 
    {
        clear();
        
        for(__uint8_t y = 0; y < 64; y++)
            for(__uint8_t x = 0; x < 240; x += 8)
                    draw_pixel(x + ((increment + y) % 8), y);

        show();
        increment = (increment + 1) % 8;
    }

    return 0;
}