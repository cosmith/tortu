#include <TFT_eSPI.h>
#include <PNGdec.h>
#include <State.h>
#include "TortuLogo.h"

#define MAX_IMAGE_WIDTH 240 // Sets rendering line buffer lengths, adjust for your images

TFT_eSPI tft = TFT_eSPI();

// Position variables must be global (PNGdec does not handle position coordinates)
int16_t xpos = 0;
int16_t ypos = 0;

PNG png; // PNG decoder instance

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

void initializeDisplay()
{
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
}

void displaySplash()
{
    tft.fillScreen(TFT_BLACK);

    uint16_t pngw = 0, pngh = 0;

    ypos = 100;
    int16_t rc = png.openFLASH((uint8_t *)TortuLogo, sizeof(TortuLogo), pngDraw);

    if (rc == PNG_SUCCESS)
    {
        pngw = png.getWidth();
        pngh = png.getHeight();
        xpos = tft.width() / 2 - pngw / 2;
        ypos = tft.height() / 2 - pngh / 2;

        tft.startWrite();
        uint32_t dt = millis();
        rc = png.decode(NULL, 0);
        tft.endWrite();
    }
}

void displayState(State state, MenuMode menuMode, JsonObject selectedItem)
{
    String name = selectedItem["name"];

    switch (state)
    {
    case INITIALIZING:
        displaySplash();
        break;
    case MENU:
        tft.fillScreen(TFT_BLACK);
        tft.drawString(name, 10, 50, 4);
        break;
    case PLAYING:
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Playing", 10, 10, 4);
        tft.drawString(name, 10, 50, 4);
        break;
    case PAUSED:
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Paused", 10, 10, 4);
        tft.drawString(name, 10, 50, 4);
        break;
    case ERROR:
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_BLACK);
        tft.drawString("Error", 10, 10, 4);
        break;
    }
}