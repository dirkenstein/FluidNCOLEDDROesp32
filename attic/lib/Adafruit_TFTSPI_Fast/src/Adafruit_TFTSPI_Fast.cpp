#include "Adafruit_TFTSPI_Fast.h"


/**************************************************************************/
/*!
   @brief      Draw a RAM-resident 1-bit image at the specified (x,y) position,
   using the specified foreground (for set bits) and background (unset bits)
   colors.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
    @param    color 16-bit 5-6-5 Color to draw pixels with
    @param    bg 16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void TFTSPI_Fast::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w,
                              int16_t h, uint16_t color, uint16_t bg) {
  
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t b = 0;
  int16_t x2, y2;                 // Lower-right coord
  if ((x >= _width) ||            // Off-edge right
      (y >= _height) ||           // " top
      ((x2 = (x + w - 1)) < 0) || // " left
      ((y2 = (y + h - 1)) < 0))
    return; // " bottom
  int16_t bx1 = 0, by1 = 0; // Clipped top-left within bitmap
  if (x < 0) {              // Clip left
    w += x;
    bx1 = -x;
    x = 0;
  }
  if (y < 0) { // Clip top
    h += y;
    by1 = -y;
    y = 0;
  }
  if (x2 >= _width)
    w = _width - x; // Clip right
  if (y2 >= _height)
    h = _height - y; // Clip bottom
  uint16_t line[w];
  startWrite();
  setAddrWindow(x, y, w, h); // Clipped area
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (i & 7)
        b <<= 1;
      else
        b = bitmap[j * byteWidth + i / 8];
      //if (b & 0x80)
        //writePixel(x + i, y, color);
      line[i] = (b & 0x80) ? color : bg;
    }
    writePixels(line, w); // Push one (clipped) row
  }
  endWrite();
}


/**************************************************************************/
/*!
   @brief      Merge multiple  RAM-resident 1-bit images at the specified (x,y) position,
   using the specified foreground (for set bits) and background (unset bits)
   colors. Later images in the array override earlier ones (only set bits from later images are coloured in).
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    nbitmaps number of bitmaps to merge
    @param    bitmaps  array of pointer to byte arrays with monochrome bitmaps
    @param    w   Width of bitmaps in pixels
    @param    h   Height of bitmap in pixels
    @param    colors array 16-bit 5-6-5 Colors to draw pixels with for each image
    @param    bg 16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void TFTSPI_Fast::drawBitmaps(int16_t x, int16_t y, int nbitmaps, uint8_t **bitmaps, int16_t w,
                              int16_t h, uint16_t *colors, uint16_t bg) {
  
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t b[nbitmaps];
  int16_t x2, y2;                 // Lower-right coord
  if ((x >= _width) ||            // Off-edge right
      (y >= _height) ||           // " top
      ((x2 = (x + w - 1)) < 0) || // " left
      ((y2 = (y + h - 1)) < 0))
    return; // " bottom
  int16_t bx1 = 0, by1 = 0; // Clipped top-left within bitmap
  if (x < 0) {              // Clip left
    w += x;
    bx1 = -x;
    x = 0;
  }
  if (y < 0) { // Clip top
    h += y;
    by1 = -y;
    y = 0;
  }
  if (x2 >= _width)
    w = _width - x; // Clip right
  if (y2 >= _height)
    h = _height - y; // Clip bottom
  uint16_t line[w];
  for (int ini = 0; ini  < nbitmaps; ini++) b[ini] = 0;
  startWrite();
  setAddrWindow(x, y, w, h); // Clipped area
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
        for (int16_t k = 0; k < nbitmaps; k++) {
          if (i & 7)
            b[k] <<= 1;
          else 
            b[k] = bitmaps[k][j * byteWidth + i / 8];
          if (k==0) line[i] = bg;
          if (b[k] & 0x80) line[i] = colors[k];
        }
    }
    writePixels(line, w); // Push one (clipped) row
  }
  endWrite();
}