#include "Status.h"

Led::Led(gpio_num_t _pin)
{
    pin = _pin;

    rmt_config_t rmt_tx;

    rmt_tx.rmt_mode = RMT_MODE_TX;
    rmt_tx.channel = RMT_CHANNEL_0;
    rmt_tx.gpio_num = pin;
    rmt_tx.mem_block_num = true;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_en = false;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    rmt_tx.clk_div = 2;

    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}
void Led::setColor(uint32_t color)
{
    uint32_t mask = 1 << (24 - 1);

    for (uint32_t bit = 0; bit < 24; bit++)
    {
        uint32_t bit_is_set = color & mask;
        led_data_buffer[24 + bit] = bit_is_set ? (rmt_item32_t){{{42, 1, 12, 0}}} : (rmt_item32_t){{{12, 1, 42, 0}}};
        mask >>= 1;
    }
    rmt_write_items(RMT_CHANNEL_0, led_data_buffer, 24, false);
    rmt_wait_tx_done(RMT_CHANNEL_0, portMAX_DELAY);
}

Relay::Relay(int _relayPin1, int _relayPin2)
{
    relayPin1 = _relayPin1;
    relayPin2 = _relayPin2;
    relay1State = false;
    relay2State = false;
}
void Relay::begin()
{
    pinMode(relayPin1, OUTPUT);
    pinMode(relayPin2, OUTPUT);
    turnOffRelay1();
    turnOffRelay2();
}
void Relay::turnOnRelay1()
{
    digitalWrite(relayPin1, HIGH);
    relay1State = true;
}
void Relay::turnOffRelay1()
{
    digitalWrite(relayPin1, LOW);
    relay1State = false;
}
bool Relay::stateRelay1()
{
    return relay1State;
}
void Relay::turnOnRelay2()
{
    digitalWrite(relayPin2, HIGH);
    relay2State = true;
}
void Relay::turnOffRelay2()
{
    digitalWrite(relayPin2, LOW);
    relay2State = false;
}
bool Relay::stateRelay2()
{
    return relay2State;
}