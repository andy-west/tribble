#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <I2S.h>

#include "tribble_angry_sound.h"
#include "tribble_coo_sound.h"

//#define DEBUG

#define VIBRATION  (12)

#define PN532_SCK  (16)
#define PN532_MOSI (18)
#define PN532_SS   (19)
#define PN532_MISO (17)
//#define PN532_IRQ  ()

#define pBCLK (8)
#define pWS   (pBCLK+1)
#define pDOUT (7)

#ifdef DEBUG
    #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
#endif

const int sampleRate = 16000;  // Minimum for UDA1334A

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
I2S i2s(OUTPUT);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(VIBRATION, OUTPUT);

    Serial.begin(9600);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);

    DEBUG_PRINTLN("Tribble connected.");

    setupNfc();
    setupI2s();
}

void setupNfc() {
    DEBUG_PRINTLN("Initializing NFC.");

    nfc.begin();
    uint32_t versionData = nfc.getFirmwareVersion();

    if (!versionData) {
        DEBUG_PRINT("PN53x board not found.");
        while (1);
    }

    DEBUG_PRINT("Found PN5 chip.");
    DEBUG_PRINTLN((versionData >> 24) & 0xFF, HEX);

    DEBUG_PRINT("Firmware ver. ");
    DEBUG_PRINT((versionData >> 16) & 0xFF, DEC);
    DEBUG_PRINT('.');
    DEBUG_PRINTLN((versionData >> 8) & 0xFF, DEC);

    nfc.setPassiveActivationRetries(0xFF);

    DEBUG_PRINTLN("Waiting for an ISO14443A card...");
}

void setupI2s() {
    DEBUG_PRINTLN("Initializing I2S.");

    i2s.setBCLK(pBCLK);
    i2s.setDATA(pDOUT);
    i2s.setBitsPerSample(16);

    if (!i2s.begin(sampleRate)) {
        DEBUG_PRINTLN("Failed to initialize I2S.");
        while (1);
    }
}

void loop() {
    tryReadCard();
}

void tryReadCard() {
    boolean success;

    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };

    // 4 or 7 bytes depending on ISO14443A card type.
    // 4 bytes = MIFARE Classic, 7 bytes = MIFARE Ultralight
    uint8_t uidLength;

    // Wait for a card.
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

    if (success) {
        DEBUG_PRINTLN("Found a card.");

        DEBUG_PRINT("UID Length: ");
        DEBUG_PRINT(uidLength, DEC);
        DEBUG_PRINTLN(" bytes");

        DEBUG_PRINT("UID Value: ");

        for (uint8_t i = 0; i < uidLength; i++)
        {
            DEBUG_PRINT(" 0x");
            DEBUG_PRINT(uid[i], HEX);
        }

        DEBUG_PRINTLN("");

        if (uid[1] % 2 == 0) {
            playAngrySound();
        } else {
            playCooSound();
        }

        delay(1000);
    }
    else
    {
        DEBUG_PRINTLN("Timed out waiting for a card.");
    }
}

void playAngrySound() {
    const int start = 0;
    const int end = 16930;

    playSound(tribbleAngrySound, start, end);
    delay(1000);
    playSound(tribbleAngrySound, start, end);
}

void playCooSound() {
    const int start1 = 0;
    const int end1 = 12320;
    const int start2 = 15376;
    const int end2 = 27390;

    playSound(tribbleCooSound, start1, end1);
    delay(190);
    playSound(tribbleCooSound, start2, end2);
    delay(1000);
    playSound(tribbleCooSound, start1, end1);
    delay(190);
    playSound(tribbleCooSound, start2, end2);
}

void playSound(uint8_t sound[], int start, int end) {
    int16_t sample;

    digitalWrite(VIBRATION, HIGH);

    for (int i = start; i < end; i++) {
        sample = static_cast<int8_t>(sound[i]) * 256 / 2;

        // Write the same sample twice, once for left and once for the right channel.
        i2s.write(sample);
        i2s.write(sample);
    }

    digitalWrite(VIBRATION, LOW);
}
