#include "displayfuncs.h"

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_TFTSPI_Fast.h"
#include <Adafruit_TSC2046.h>

#define DISPLAY_HEIGHT ILI9341_TFTHEIGHT
#define DISPLAY_WIDTH ILI9341_TFTWIDTH

class TFTSPIFastDisplayFunctions :  public DisplayFunctions {
public:
    TFTSPIFastDisplayFunctions(int rst=2, int cs=SS, int dc=13);
    Adafruit_TSC2046 touchscreen;

private: 
    SPIClass myspi;
    TFTSPI_Fast tft;
    GFXcanvas1 canvas[3];
    int _currcanvas = 0;
    uint16_t foregnd_colour = ILI9341_WHITE;

    uint16_t backgnd_colour = ILI9341_BLACK;
    uint16_t _palette[3];
    uint16_t linespc = 3; 
public:
    void begin() override;
    void updateDisplay() override;
    void showFileDisplay() override;
    void showRadioDisplay() override;
    void showBootLogo() override;
    void showWebUIDisplay() override;
    void drawProgressBar(int x, int y, int width, int height, int percent) override;
    void drawCheckbox(int x, int y, int width, bool checked, std::string label) override;
    void drawStr(int x, int y, const char * str);
 //   void drawFrame(int x, int y, int width, int height);       
};