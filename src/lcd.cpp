#include <lcd.h>
#include "pico/stdlib.h"

#define DELAY 5

namespace GameStation
{
    uint8_t LCD::s_hn;
    uint16_t LCD::s_scrnw, LCD::s_scrnh;
    
    uint8_t LCD::s_rw, LCD::s_rs, LCD::s_en, LCD::s_rst, LCD::s_cs;
    std::array<uint8_t, 8> LCD::s_data_pins;

    void LCD::init(const uint16_t scrn_w, const uint16_t scrn_h, const LCDPinsConfiguration &pins)
    {
        s_scrnw = scrn_w;
        s_scrnh = scrn_w;
        s_hn = s_scrnw / 8;

        s_rw = pins.get_rw_pin();
        s_rs = pins.get_rs_pin();
        s_en = pins.get_en_pin();
        s_rst = pins.get_rst_pin();
        s_cs = pins.get_cs_pin();
        s_data_pins = pins.get_data_pins();

        // Iniciando portas do RP2040
        gpio_init(s_rs);
        gpio_set_dir(s_rs, GPIO_OUT);

        gpio_init(s_rw);
        gpio_set_dir(s_rw, GPIO_OUT);

        gpio_init(s_en);
        gpio_set_dir(s_en, GPIO_OUT);

        gpio_init(s_cs);
        gpio_set_dir(s_cs, GPIO_OUT);

        gpio_init(s_rst);
        gpio_set_dir(s_rst, GPIO_OUT);

        for (__uint8_t i = 0; i < s_data_pins.size(); i++)
        {
            gpio_init(s_data_pins[i]);
            gpio_set_dir(s_data_pins[i], GPIO_OUT);
        }

        // Resetando e selecionando o LCD
        gpio_put(s_rst, false);
        sleep_ms(1000); // TODO: Verificar se pode diminuir esse tempo de reset
        gpio_put(s_rst, true);
        sleep_ms(1000);
        gpio_put(s_cs, false);

        // Define para usar o modo gráfico do LCD
        send_command(CommandType::Mode, 0b00100010);

        // Define quantos dots 1 byte equivale, por padrão
        // Defini para que 8 bits sejam 8 dots
        send_command(CommandType::HP, 0b00000111);

        // Como o HP foi definido como 8 bits para 8 dots aqui deve ser
        // Quantos bytes tem uma linha inteira então se a tela tem
        // 240 dots horizontais enviamos (240 / HP) - 1
        send_command(CommandType::HN, s_hn - 1);

        // Número de dots verticais - 1
        send_command(CommandType::NX, scrn_h - 1);

        send_command(CommandType::CURSOR, 0);

        send_command(CommandType::LOWER_DISPLAY, 0);
        send_command(CommandType::UPPER_DISPLAY, 0);

        send_command(CommandType::LOWER_CURSOR, 0);
        send_command(CommandType::UPPER_CURSOR, 0);
    }

    void LCD::write_byte(const uint8_t byte)
    {
        for(uint32_t i = 0; i < s_data_pins.size(); i++)
            gpio_put(s_data_pins[i], (byte & (1 << i)));
    }

    void LCD::send_command(const CommandType command, const uint8_t data)
    {
        gpio_put(s_rw, false);
        gpio_put(s_rs, true);
        gpio_put(s_en, true);
        write_byte(command);
        gpio_put(s_en, false);

        // TODO: Substituir os dois delays por interrupção
        sleep_us(DELAY);
        
        gpio_put(s_rw, false);
        gpio_put(s_rs, false);
        gpio_put(s_en, true);
        write_byte(data);
        gpio_put(s_en, false);

        sleep_us(DELAY);
    }

    void LCD::write_screen_buffer(const uint8_t *screen_buffer)
    {
        // Resetanto cursor
        send_command(CommandType::LOWER_CURSOR, 0);
        send_command(CommandType::UPPER_CURSOR, 0);

        for(uint32_t i = 0; i < s_hn * s_scrnh; i++)
            send_command(CommandType::WRITE_HLINE, screen_buffer[i]);
    }
}
