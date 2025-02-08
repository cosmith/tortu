#include "SPI.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

#include <SDFS.h>
#include <ArduinoJson.h>

#include <AudioGeneratorMP3.h>
#include <AudioFileSourceFS.h>
#include <AudioFileSourceBuffer.h>
#include <AudioOutputI2S.h>

#include <Display.h>
#include <Button.h>
#include <SDUtils.h>

// SD card
#define SD_SPI_CLK 10
#define SD_SPI_MOSI 11
#define SD_SPI_MISO 12
#define SD_SPI_CS 13

// Test with reduced SPI speed for breadboards.  SD_SCK_MHZ(4) will select
// the highest speed supported by the board that is not over 4 MHz.
// Change SPI_SPEED to SD_SCK_MHZ(50) for best performance.
#define SPI_SPEED SD_SCK_MHZ(10)

// Audio
#define AUDIO_I2S_BCLK 7
#define AUDIO_I2S_LRC 8
#define AUDIO_I2S_DOUT 9

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

// State management
State state = INITIALIZING;
State previousState = MENU;
MenuMode menuMode = STORIES;
MenuMode previousMenuMode = STORIES;
int selectedItemIndex = 0;
int previousSelectedItemIndex = 0;
JsonDocument config;

void play(const char *path)
{
    Serial.printf("Playing %s\n", path);
    audioFileSourceFS = new AudioFileSourceFS(SDFS, path);
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

// String input;
bool readConfig()
{
    File f = SDFS.open("/tortu.json", "r");
    Serial.print(f.size());

    DeserializationError error = deserializeJson(config, f);

    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return false;
    }

    return true;
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
    Serial.print("Files in /: ");
    Serial.println(dir.next() ? "Yes" : "No");

    // Start the recursive listing from the root directory
    // Only for debugging purposes
    // listDirectoryContents("/");

    bool configOK = readConfig();

    if (!configOK)
    {
        state = ERROR;
        return;
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

    state = MENU;
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

    String dir = menuMode == MUSIC ? "music" : "stories";
    JsonObject selectedItem = config[dir][selectedItemIndex];

    switch (state)
    {
    case MENU:
        delay(16);

        if (playPressed)
        {
            String path = "/" + dir + "/" + selectedItem["mp3"].as<String>();
            play(path.c_str());
        }

        if (leftPressed)
        {
            selectedItemIndex = (selectedItemIndex - 1) % config[dir].size();
        }

        if (rightPressed)
        {
            selectedItemIndex = (selectedItemIndex + 1) % config[dir].size();
        }

        if (storiesPressed && menuMode == MUSIC)
        {
            menuMode = STORIES;
            selectedItemIndex = 0;
        }

        if (musicPressed && menuMode == STORIES)
        {
            menuMode = MUSIC;
            selectedItemIndex = 0;
        }

        break;
    case PLAYING:
        playingAudio = audioGeneratorMP3->loop();

        if (!playingAudio)
        {
            Serial.println("Audio finished");
            stop();
        }
        if (playPressed)
        {
            Serial.println("Play pressed, pausing");
            pause();
        }
        if (musicPressed)
        {
            Serial.println("Music pressed, stopping");
            stop();
            if (menuMode == STORIES)
            {
                menuMode = MUSIC;
                selectedItemIndex = 0;
            }
        }
        if (storiesPressed)
        {
            Serial.println("Stories pressed, stopping");
            stop();
            if (menuMode == MUSIC)
            {
                menuMode = STORIES;
                selectedItemIndex = 0;
            }
        }
        if (rightPressed)
        {
            stop();
            selectedItemIndex = (selectedItemIndex + 1) % config[dir].size();
        }
        if (leftPressed)
        {
            stop();
            selectedItemIndex = (selectedItemIndex - 1) % config[dir].size();
        }
        break;
    case PAUSED:
        delay(16);

        if (playPressed)
        {
            state = PLAYING;
        }
        if (musicPressed || storiesPressed)
        {
            Serial.println("Music/stories pressed, stopping");
            stop();
        }
        break;
    }
}

void loop1()
{
    String dir = menuMode == MUSIC ? "music" : "stories";
    JsonObject selectedItem = config[dir][selectedItemIndex];

    if (previousState != state || previousMenuMode != menuMode || previousSelectedItemIndex != selectedItemIndex)
    {
        Serial.printf("State changed from %d to %d\n", previousState, state);
        previousState = state;
        previousMenuMode = menuMode;
        previousSelectedItemIndex = selectedItemIndex;
        displayState(state, menuMode, selectedItem);
    }

    delay(16);
}
