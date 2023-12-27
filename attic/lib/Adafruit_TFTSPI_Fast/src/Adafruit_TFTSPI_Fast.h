
#pragma once
#include <Adafruit_ILI9341.h>

class TFTSPI_Fast : public Adafruit_ILI9341 {
public:
  TFTSPI_Fast(SPIClass *spiClass, int8_t dc, int8_t cs = -1,
                   int8_t rst = -1) : Adafruit_ILI9341(spiClass, dc, cs, rst) {};
  using Adafruit_GFX::drawBitmap;
  void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w,
                              int16_t h, uint16_t color, uint16_t bg);
  void drawBitmaps(int16_t x, int16_t y, int nbitmaps, uint8_t **bitmaps, int16_t w,
                              int16_t h, uint16_t *colors, uint16_t bg);
};

