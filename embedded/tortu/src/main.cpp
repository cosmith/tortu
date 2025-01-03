#include "SPI.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

#include <SDFS.h>

#include <AudioGeneratorMP3.h>
#include <AudioFileSourceFS.h>
#include <AudioFileSourceBuffer.h>
#include <AudioOutputI2S.h>

#include <Display.h>
#include <Button.h>

// SD card
#define SD_SPI_CLK 10
#define SD_SPI_MOSI 11
#define SD_SPI_MISO 12
#define SD_SPI_CS 13

// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(2)

// Audio
#define AUDIO_I2S_BCLK 0
#define AUDIO_I2S_LRC 1
#define AUDIO_I2S_DOUT 2

AudioGeneratorMP3 *audioGeneratorMP3;
AudioFileSourceBuffer *audioFileSourceBuffer;
AudioFileSourceFS *audioFileSourceFS;
AudioOutputI2S *audioOutput;

// Buttons
Button buttonLeft(16);
Button buttonPlay(22);
Button buttonMusic(26);
Button buttonStories(27);
Button buttonRight(28);

State state = MENU;
State previousState = MENU;

void play(const char *filename)
{
    Serial.printf("MP3 play\n");
    audioFileSourceFS = new AudioFileSourceFS(SDFS, filename);
    audioGeneratorMP3 = new AudioGeneratorMP3();
    audioGeneratorMP3->begin(audioFileSourceFS, audioOutput);
    state = PLAYING;
}

void pause()
{
    Serial.printf("MP3 pause\n");
    state = PAUSED;
}

void stop()
{
    Serial.printf("MP3 stop\n");
    audioGeneratorMP3->stop();
    state = MENU;
}

void setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.println("Serial started");

    // Initialize SD card
    SPI1.setRX(SD_SPI_MISO);
    SPI1.setTX(SD_SPI_MOSI);
    SPI1.setSCK(SD_SPI_CLK);
    SPI1.setCS(SD_SPI_CS);

    Serial.println("SPI pins set");

    SDFSConfig sdFsConfig;
    sdFsConfig.setSPISpeed(SPI_SPEED);
    sdFsConfig.setSPI(SPI1);

    Serial.println("SDFS config set");

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

    audioOutput = new AudioOutputI2S(44100);
    audioOutput->SetPinout(AUDIO_I2S_BCLK, AUDIO_I2S_LRC, AUDIO_I2S_DOUT);
    audioOutput->SetOutputModeMono(true);
    audioOutput->SetGain(1);

    // Initialize buttons
    buttonLeft.begin();
    buttonPlay.begin();
    buttonMusic.begin();
    buttonStories.begin();
    buttonRight.begin();

    play("/lion44-96mono.mp3");

    Serial.println("\r\nInitialisation done.");
}

void setup1()
{
    initializeDisplay();
    displaySplash();
}

void loop()
{
    bool playPressed = buttonPlay.pressed();
    bool musicPressed = buttonMusic.pressed();
    bool storiesPressed = buttonStories.pressed();
    bool rightPressed = buttonRight.pressed();
    bool leftPressed = buttonLeft.pressed();

    bool playingAudio = false;

    switch (state)
    {
    case MENU:
        delay(16);

        if (playPressed)
        {
            play("/lion44-96mono.mp3");
        }

        if (leftPressed)
        {
            play("/lion44-96mono.mp3");
        }

        if (rightPressed)
        {
            play("/lion44-96mono.mp3");
        }
        break;
    case PLAYING:
        playingAudio = audioGeneratorMP3->loop();

        if (!playingAudio)
        {
            Serial.printf("Audio finished\n");
            stop();
        }
        if (playPressed)
        {
            Serial.printf("Play pressed, pausing\n");
            pause();
        }
        if (musicPressed || storiesPressed)
        {
            Serial.printf("Music/stories pressed, stopping\n");
            stop();
        }
        break;
    case PAUSED:
        if (playPressed)
        {
            state = PLAYING;
        }
        if (musicPressed || storiesPressed)
        {
            Serial.printf("Music/stories pressed, stopping\n");
            stop();
        }
        break;
    }
}

void loop1()
{
    bool left = buttonLeft.pressed();
    bool play = buttonPlay.pressed();
    bool music = buttonMusic.pressed();
    bool stories = buttonStories.pressed();
    bool right = buttonRight.pressed();

    if (previousState != state)
    {
        Serial.printf("State changed from %d to %d\n", previousState, state);
        previousState = state;
        displayState(state);
    }

    delay(16);
}
