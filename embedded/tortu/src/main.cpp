#include "SPI.h"
#include <Arduino.h>
#include <PNGdec.h>
#include <TFT_eSPI.h>

#include <SDFS.h>

#include <AudioGeneratorMP3.h>
#include <AudioFileSourceFS.h>
#include <AudioFileSourceBuffer.h>
#include <AudioOutputI2S.h>

#include "turtle.h"

PNG png; // PNG decoder instance

#define MAX_IMAGE_WIDTH 240 // Sets rendering line buffer lengths, adjust for your images

TFT_eSPI tft = TFT_eSPI();

// Position variables must be global (PNGdec does not handle position coordinates)
int16_t xpos = 0;
int16_t ypos = 0;

void pngDraw(PNGDRAW *pDraw)
{
    uint16_t lineBuffer[MAX_IMAGE_WIDTH];
    uint8_t pMask[40];

    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
    if (png.getAlphaMask(pDraw, pMask, 200))
    { // if any pixels are opaque, draw them
        tft.pushMaskedImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer, pMask);
    }
}

// SD card
#define SD_SPI_CLK 10
#define SD_SPI_MOSI 11
#define SD_SPI_MISO 12
#define SD_SPI_CS 13

#define SD_SPI_FREQ 40000000

// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(4)

// Audio
#define AUDIO_I2S_BCLK 0
#define AUDIO_I2S_LRC 1
#define AUDIO_I2S_DOUT 2

AudioGeneratorMP3 *audioGeneratorMP3;
AudioFileSourceBuffer *audioFileSourceBuffer;
AudioFileSourceFS *audioFileSourceFS;
AudioOutputI2S *audioOutput;

void setup()
{
    Serial.begin(115200);
    // delay(5000);
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
    const char *song = "/lion44-56mono.mp3";

    audioOutput = new AudioOutputI2S();
    audioOutput->SetPinout(AUDIO_I2S_BCLK, AUDIO_I2S_LRC, AUDIO_I2S_DOUT);

    audioLogger = &Serial;
    audioFileSourceFS = new AudioFileSourceFS(SDFS, song);

    audioGeneratorMP3 = new AudioGeneratorMP3();

    Serial.printf("BEGIN...\n");
    audioGeneratorMP3->begin(audioFileSourceFS, audioOutput);

    Serial.println("\r\nInitialisation done.");
}

void setup1()
{
    // Initialize the screen
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(0x0000);
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
    uint16_t pngw = 0, pngh = 0; // To store width and height of image

    int16_t rc = png.openFLASH((uint8_t *)turtle, sizeof(turtle), pngDraw);

    if (rc == PNG_SUCCESS)
    {
        Serial.println("Successfully opened png file");
        pngw = png.getWidth();
        pngh = png.getHeight();

        tft.startWrite();
        uint32_t dt = millis();
        rc = png.decode(NULL, 0);
        tft.endWrite();
        Serial.print(millis() - dt);
        Serial.println("ms");
        tft.endWrite();

        // png.close(); // Required for files, not needed for FLASH arrays
    }

    delay(100);

    // Randomly change position
    xpos = random(tft.width() - pngw);
    ypos = random(tft.height() - pngh);
}
