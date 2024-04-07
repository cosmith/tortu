#include "SPI.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

#include <SDFS.h>

#include <AudioGeneratorMP3.h>
#include <AudioFileSourceFS.h>
#include <AudioFileSourceBuffer.h>
#include <AudioOutputI2S.h>

#include <Display.h>

// SD card
#define SD_SPI_CLK 10
#define SD_SPI_MOSI 11
#define SD_SPI_MISO 12
#define SD_SPI_CS 13

// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(16)

// Audio
#define AUDIO_I2S_BCLK 0
#define AUDIO_I2S_LRC 1
#define AUDIO_I2S_DOUT 2

AudioGeneratorMP3 *audioGeneratorMP3;
AudioFileSourceBuffer *audioFileSourceBuffer;
AudioFileSourceFS *audioFileSourceFS;
AudioOutputI2S *audioOutput;

enum State
{
    MENU,
    PLAYING,
    PAUSED,
};

State state = MENU;

void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("Welcome to Tortu :)");

    // Initialize SD card
    SPI1.setRX(SD_SPI_MISO);
    SPI1.setTX(SD_SPI_MOSI);
    SPI1.setSCK(SD_SPI_CLK);
    SPI1.setCS(SD_SPI_CS);
    SDFSConfig sdFsConfig;
    sdFsConfig.setSPI(SPI1);
    SDFS.setConfig(sdFsConfig);
    bool sdInit = SDFS.begin();
    Serial.println(sdInit ? "SD card initialized" : "SD card initialization failed");

    Dir dir = SDFS.openDir("/");
    Serial.println("Files in /:");
    Serial.println(dir.next() ? "Yes" : "No");
    while (dir.next())
    {
        Serial.print(dir.fileName() + " - ");
        if (dir.fileSize())
        {
            File f = dir.openFile("r");
            Serial.println(f.size());
        }
    }

    // Initialize audio I2S interface
    const char *song = "/baila-44100-128-compressed.mp3";

    audioOutput = new AudioOutputI2S(44100);
    audioOutput->SetPinout(AUDIO_I2S_BCLK, AUDIO_I2S_LRC, AUDIO_I2S_DOUT);
    audioOutput->SetOutputModeMono(true);
    audioOutput->SetGain(0.7);

    audioLogger = &Serial;
    audioFileSourceFS = new AudioFileSourceFS(SDFS, song);

    audioGeneratorMP3 = new AudioGeneratorMP3();

    Serial.printf("BEGIN...\n");
    audioGeneratorMP3->begin(audioFileSourceFS, audioOutput);

    Serial.println("\r\nInitialisation done.");
}

void setup1()
{
    initializeDisplay();
    displaySplash();
}

void loop()
{
    if (audioGeneratorMP3->isRunning())
    {
        if (!audioGeneratorMP3->loop())
        {
            audioGeneratorMP3->stop();
        }
    }
    else
    {
        Serial.printf("MP3 done\n");
        delay(1000);
    }
}

void loop1()
{
}
