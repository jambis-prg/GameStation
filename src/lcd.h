#pragma once
#include <cstdint>
#include <array>

namespace GameStation
{
    enum CommandType : uint8_t
    {
        Mode            = 0x00,
        HP              = 0x01,
        HN              = 0x02,
        NX              = 0x03,
        CURSOR          = 0x04,
        LOWER_DISPLAY   = 0x08,
        UPPER_DISPLAY   = 0x09,
        LOWER_CURSOR    = 0x0A,
        UPPER_CURSOR    = 0x0B,
        WRITE_HLINE     = 0x0C
    };

    class LCDPinsConfiguration
    {
    private:
        uint8_t m_rw, m_rs, m_en, m_rst, m_cs;
        std::array<uint8_t, 8> m_data_pins;
    public:
        LCDPinsConfiguration(uint8_t rw, uint8_t rs, uint8_t en, uint8_t rst, uint8_t cs, std::array<uint8_t, 8> data_pins) :
        m_rw(rw), m_rs(rs), m_en(en), m_rst(rst), m_cs(cs)
        {
            m_data_pins = data_pins;
        }

        LCDPinsConfiguration(LCDPinsConfiguration &other) : 
        m_rw(other.m_rw), m_rs(other.m_rs), m_en(other.m_en), m_rst(other.m_rst), m_cs(other.m_cs)
        {
            m_data_pins = other.m_data_pins;
        }

        uint8_t get_rw_pin() const { return m_rw; }
        uint8_t get_rs_pin() const { return m_rs; }
        uint8_t get_en_pin() const { return m_en; }
        uint8_t get_rst_pin() const { return m_rst; }
        uint8_t get_cs_pin() const { return m_cs; }
        const std::array<uint8_t, 8>& get_data_pins() const { return m_data_pins; }

        LCDPinsConfiguration& operator=(LCDPinsConfiguration &other)
        {
            m_rw = other.m_rw;
            m_rs = other.m_rs;
            m_en = other.m_en;
            m_rst = other.m_rst;
            m_cs = other.m_cs;
            m_data_pins = other.m_data_pins;

            return *this;
        }
    };

    class LCD
    {
    private:
        static uint8_t s_hn;
        static uint16_t s_scrnw, s_scrnh;

        // Pinos
        static uint8_t s_rw, s_rs, s_en, s_rst, s_cs;
        static std::array<uint8_t, 8> s_data_pins;

        static void write_byte(const uint8_t byte);
        static void send_command(const CommandType command, const uint8_t data);
    public:
        static void init(const uint16_t scrn_w, const uint16_t scrn_h, const LCDPinsConfiguration &pins);
        static void write_screen_buffer(const uint8_t *screen_buffer);

        static uint8_t get_hn() { return s_hn; }
    };
}
