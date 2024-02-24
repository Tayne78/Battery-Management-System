#include "OneWireCom.h"

volatile bool OneWireCom::dataReceived = false;
volatile unsigned long OneWireCom::lastChangeTime = 0;

OneWireCom::OneWireCom(gpio_num_t _txPin, gpio_num_t _rxPin)
{
    txPin = _txPin;
    rxPin = _rxPin;

    rmt_config_t rmt_tx, rmt_rx;

    rmt_tx.rmt_mode = RMT_MODE_TX;
    rmt_tx.channel = RMT_CHANNEL_0;
    rmt_tx.gpio_num = txPin;
    rmt_tx.mem_block_num = true;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_en = false;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    rmt_tx.clk_div = 80;

    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);

    rmt_rx.channel = RMT_CHANNEL_1;
    rmt_rx.gpio_num = rxPin;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.clk_div = 80;
    rmt_rx.mem_block_num = 4;
    rmt_rx.rx_config.filter_en = true;
    rmt_rx.rx_config.filter_ticks_thresh = 200;
    rmt_rx.rx_config.idle_threshold = 3000;

    rmt_config(&rmt_rx);
    rmt_driver_install(rmt_rx.channel, 4000, 0);

    rmt_get_ringbuf_handle(rmt_rx.channel, &rb);
    rmt_rx_start(rmt_rx.channel, 1);
}
bool OneWireCom::send(uint16_t data)
{
    rmt_item32_t manchester_data[16];

    uint8_t highByte = (uint8_t)(data >> 8) | 0x80;
    uint8_t lowByte = (uint8_t)(data & 0x00ff);

    data = 0;

    for (int i = 0; i < 8; i++)
    {
        data |= (((lowByte ^ 0xFF) >> i) & 1) << (15 - i);
        data |= (((highByte ^ 0xFF) >> i) & 1) << (7 - i);
    }

    bool bit;

    for (int i = 0; i < 16; i++)
    {
        bit = (data >> i) & 1;
        if (bit == 1)
        {
            manchester_data[i].level0 = 0;
            manchester_data[i].duration0 = 210;
            manchester_data[i].level1 = 1;
            manchester_data[i].duration1 = 210;
        }
        else
        {
            manchester_data[i].level0 = 1;
            manchester_data[i].duration0 = 210;
            manchester_data[i].level1 = 0;
            manchester_data[i].duration1 = 210;
        }
    }

    rmt_write_items(RMT_CHANNEL_0, manchester_data, 16, true);
    if (rmt_wait_tx_done(RMT_CHANNEL_0, portMAX_DELAY) != ESP_OK)
    {
        return 0; // Fehler: Senden fehlgeschlagen
    }
    return 1;
}

void OneWireCom::begin()
{
    attachInterrupt(digitalPinToInterrupt(rxPin), receiveData, CHANGE);
}
void IRAM_ATTR OneWireCom::receiveData()
{    
    if (!dataReceived)
    {
        dataReceived = true;
        lastChangeTime = millis();
    }
  
}
bool OneWireCom::getstatus()
{
    return dataReceived && (millis() - lastChangeTime > 8);
}
void OneWireCom::setstatus()
{
    dataReceived = false;
}
void OneWireCom::ringbufferclear() {
    size_t rx_size = 0;
    rmt_item32_t* items = (rmt_item32_t*)xRingbufferReceive(rb, &rx_size, 0);
    if (items != NULL) {
        vRingbufferReturnItem(rb, (void*)items);
    }
}
u_int16_t OneWireCom::receive()
{
    size_t rx_size = 0;
    rmt_item32_t *item = (rmt_item32_t*)xRingbufferReceive(rb, &rx_size, 0);
    if (item == NULL) {
        return 0; // Fehler: Keine Daten erhalten
        Serial.print("Keine Daten verfügbar");
    }

    u_int64_t databit = 0;
    u_int16_t data = 0;
    uint8_t correction = 0;

    rmt_item32_t *it = item;
    for (uint32_t i = 0; i < rx_size / sizeof(rmt_item32_t); i++)
    {
        /*Serial.print("Level0:");
            Serial.println(it->level0);
            Serial.print("Duration0:");
            Serial.println(it->duration0);
            Serial.print("Level1:");
            Serial.println(it->level1);
            Serial.print("Duration1:");
            Serial.println(it->duration1);*/
        if ((it->duration0 > 250) && (it->duration1 <= 250))
        {
            databit = (databit << 1) | it->level0;
            databit = (databit << 1) | it->level0;
            databit = (databit << 1) | it->level1;
            correction += 3;
        }
        else if ((it->duration0 <= 250) && (it->duration1 > 250))
        {
            databit = (databit << 1) | it->level0;
            databit = (databit << 1) | it->level1;
            databit = (databit << 1) | it->level1;
            correction += 3;
        }
        else if ((it->duration0 > 250) && (it->duration1 > 250))
        {
            databit = (databit << 1) | it->level0;
            databit = (databit << 1) | it->level0;
            databit = (databit << 1) | it->level1;
            databit = (databit << 1) | it->level1;
            correction += 4;
        }
        else
        {
            databit = (databit << 1) | it->level0;
            databit = (databit << 1) | it->level1;
            correction += 2;
        }

        it++;
    }
    /*Serial.println(correction);
    Serial.println(databit,BIN);*/

    vRingbufferReturnItem(rb, (void *)item);

    if (correction == 33 && (databit & 0x01))
        databit >>= 1;
    else if (correction == 32)
    {
        for (int i = 0; i < 32; i += 2)
        {
            uint8_t bit1 = (databit >> i) & 0x01;
            uint8_t bit2 = (databit >> (i + 1)) & 0x01;

            if (!((bit1 == 0 && bit2 == 1) || (bit1 == 1 && bit2 == 0)))
            {
                databit >> 1;
                break;
            }
        }
    }
    else if (correction != 0)
    {
        Serial.println("ERROR");
        return 0; // Fehler: Korrektur unvollständig oder inkorrekt
    }

    for (int i = 0; i < 32; i += 2)
    {
        if (((databit >> i) & 0x01) == 1 && ((databit >> (i + 1)) & 0x01) == 0)
            data = (data << 1) | 0;
        else if (((databit >> i) & 0x01) == 0 && ((databit >> (i + 1)) & 0x01) == 1)
            data = (data << 1) | 1;
        else
        {
            return 0; // Fehler: Daten ungültig
            Serial.println("ERROR");
        }
    }
    uint8_t highByte = (uint8_t)(data >> 8);
    uint8_t lowByte = (uint8_t)(data & 0x00ff);

    data = 0;

    for (int i = 0; i < 8; i++)
    {
        data |= (((lowByte ^ 0xFF) >> i) & 1) << (15 - i);
        data |= (((highByte ^ 0xFF) >> i) & 1) << (7 - i);
    }
    data &= 0x7FFF;

    return data;
}
