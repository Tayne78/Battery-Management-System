#ifndef OneWireCom_h
#define OneWireCom_h

#include <Arduino.h>
#include <driver/rmt.h>

#define REQ_TEMP_G 0xD4AA   //global request for temperature 1 10 10101 0 0101010
#define REQ_VOLT_G 0xC633    //global request for voltage 1 10 00110 0 0110011
#define COM_SLP_G 0xFF0F    //global command for sleep 1 11 1111 1 00001111
#define COM_BLC_OFF_G 0xB955 //global command balancer off
#define COM_BLC_A 0xA600    //adressed command for balancing 1 01 00110 0 0000000 COM_BLC_A + adress = data

class OneWireCom
{
public:
    // Konstruktor, initialisiert die Klasse mit den Pins für Senden und Empfangen
    OneWireCom(gpio_num_t _txPin, gpio_num_t _rxPin);
    //GIPO-Pins Senden / Empfangen

    // Sendet Daten mithilfe der Manchester-Codierung
    // Gibt bei Fehler des Senden '0' zurück, wenn nicht '1'
    bool send(uint16_t data);

    // Initialisiert die Konfiguration für das Empfangen von Daten
    void begin(void);

    // Empfängt Daten mithilfe der Manchester-Dekodierung
    // Gibt Empfangene Daten zurück
    u_int16_t receive(void);

    // Gibt den aktuellen Status des Empfangs zurück (ob Daten empfangen wurden oder nicht)
    // Gibt '1' zurück, wenn Daten empfangen wurden, '0' wenn nicht
    bool getstatus(void); 

    // Setzt den Status des Empfangs zurück
    void setstatus(void);

    void ringbufferclear(void);


private:
    gpio_num_t txPin;
    gpio_num_t rxPin;

    RingbufHandle_t rb = NULL;

    static volatile unsigned long lastChangeTime;

    static volatile bool dataReceived;
    static void IRAM_ATTR receiveData();
};

#endif
