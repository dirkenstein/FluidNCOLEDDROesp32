#include "TFTSPIFastdisplayfuncs.h"
#include "fnc.h"
#include "pin_config.h"
#include "Fonts/FreeSans18pt7b.h"

#define TS_RESISTANCE 400

TFTSPIFastDisplayFunctions::TFTSPIFastDisplayFunctions(int rst, int cs, int dc) : myspi(HSPI), tft{&myspi, dc, -1, rst}, 
        canvas{{tft.height(), tft.width()},{tft.height(), tft.width()}, {tft.height(), tft.width()} },
        _palette{backgnd_colour,backgnd_colour, backgnd_colour} 
{ 
        
        clearReportData();
};

void TFTSPIFastDisplayFunctions::begin()  { 
        myspi.begin(PIN_DISPLAY_SCLK, PIN_DISPLAY_MISO, PIN_DISPLAY_MOSI, PIN_DISPLAY_CS);
        pinMode (PIN_DISPLAY_CS, OUTPUT);
        pinMode(PIN_DISPLAY_LCD, OUTPUT);
        ledcSetup(0, 5000, 8);
        ledcAttachPin(PIN_DISPLAY_LCD, 0);
        ledcWrite(0, 64);
        touchscreen.begin(PIN_TSC_CS, &myspi, TS_RESISTANCE);
        touchscreen.enableInterrupts(true);
        touchscreen.setTouchedThreshold(10000.0f);  

        //tft.swapBytes();   //u8g2.setRotation(3);
        //tft.setSPISpeed(40000000L);
        tft.begin();
        tft.setSPISpeed(40000000L);
        tft.setRotation(3);
   
        for (int x = 0; x < 3; x++) {
            canvas[x].setRotation(0);
            //tft.swapBytes();
            //sprite1.setColorDepth(16);
            //sprite1.createSprite(DISP_WIDTH, DISP_HEIGHT);
            //sprite1.fillSprite(backgnd_colour);
            canvas[x].fillScreen(backgnd_colour);

            //u8g2_prepare();

            canvas[x].setFont(&FreeSans18pt7b);
            canvas[x].setTextWrap(false);
            canvas[x].setTextColor(foregnd_colour, backgnd_colour);
            //canvas[x].setDrawColor(foregnd_colour, backgnd_colour);
            //canvas.setTextSize(4);
        }
        _palette[0] = foregnd_colour;
        /* tft.setFontRefHeightExtendedText();
        tft.setFontMode(0);
        tft.setFontPosTop(); 
        tft.setFontDirection(0); */
        //canvas.drawRect(0, 0, DISP_WIDTH, DISP_HEIGHT);
        //canvas.setCursor(0, 0);
        //canvas.print("FluidNC Channel pendant");
        //tft.drawRGBBitmap(0,0, canvas.getBuffer(), canvas.width(), canvas.height());

}
void TFTSPIFastDisplayFunctions::drawStr(int x, int y, const char * str) {
    int16_t x1, y1;
    uint16_t fh,fw;
    canvas[_currcanvas].getTextBounds(str,x, y,&x1, &y1, &fw, &fh);
    canvas[_currcanvas].setCursor(x, y + fh -1);
    canvas[_currcanvas].print(str);
}

/* void ILI9431DisplayFunctions::drawFrame(int x, int y, int width, int height) {
    canvas[_currcanvas].drawLine(x, y, x+width, y, foregnd_colour);
    canvas[_currcanvas].drawLine(x, y, x, y+height, foregnd_colour);
    canvas[_currcanvas].drawLine(x+width, y, x+width, y+height, foregnd_colour);
    canvas[_currcanvas].drawLine(x, y+height, x+width, y+height, foregnd_colour);
} */

void TFTSPIFastDisplayFunctions::drawProgressBar(int x, int y, int width, int height, int percent) {
    canvas[_currcanvas].drawRect(x, y, width, height, foregnd_colour);
    canvas[_currcanvas].fillRect(x, y, (uint32_t)(width*100)/percent, height, foregnd_colour); 
}


void TFTSPIFastDisplayFunctions::drawCheckbox(int x, int y, int width, bool checked, std::string label) {
    if (checked) {
        canvas[_currcanvas].fillRect(x, y, width, width, foregnd_colour);
    } else {
        canvas[_currcanvas].drawRect(x, y, width, width, foregnd_colour);
    }
//    int16_t x1, y1;
//    uint16_t fh,fw;
//    tft.getTextBounds(label.c_str(), x, y,&x1, &y1, &fw, &fh);
      drawStr(x + width + 5, y, label.c_str());
}

void TFTSPIFastDisplayFunctions::showBootLogo() {
    tft.fillScreen(backgnd_colour);
    const int logo_height = 170;
    const int logo_width = 320;
    Serial.println("Boot Logo");
    tft.drawRGBBitmap(0, (tft.height()-logo_height)/2, logo, logo_width, logo_height);
    int16_t x1, y1;
    uint16_t fh,fw;
    tft.getTextBounds(version.c_str(),0, 0,&x1, &y1, &fw, &fh);
    //drawStr(0, canvas.height() -fh, version.c_str());
    tft.setCursor(0, tft.height());
    tft.print(version.c_str());
}

void TFTSPIFastDisplayFunctions::updateDisplay() {
    char   buf[12];
    int cboxofs; 
    std::string axesNames = "XYZABC";
    //tft.prepare();
    //sprite1.fillSprite(backgnd_colour);
    canvas[0].fillScreen(backgnd_colour);
    canvas[1].fillScreen(backgnd_colour);
    _palette[0] = foregnd_colour;
    if (myState == "Alarm") {
        canvas[1].setTextColor(ILI9341_RED, backgnd_colour);
        _palette[1] = ILI9341_RED;
        //tft.setDrawColor(0);
    } else if (myState.find("Hold") == 0) {
         //tft.setDrawColor(0);
        canvas[1].setTextColor(ILI9341_YELLOW, backgnd_colour);
        _palette[1] = ILI9341_YELLOW;
    } else {
        //tft.setDrawColor(1);
        canvas[1].setTextColor(ILI9341_GREEN, backgnd_colour);
        _palette[1] = ILI9341_GREEN;
    }
    //int  lnHeight = tft.getMaxCharHeight();
    int16_t x1, y1;
    uint16_t fh,fw;
    const char * prst = "Probe"; 
    canvas[1].getTextBounds(myState.c_str(),0, 0,&x1, &y1, &fw, &fh);
    //Serial.printf("X1: %d Y1: %d W:%d H:%d", x1, y1, fw, fh);
    _currcanvas = 1;
    drawStr(0, 0, myState.c_str());
    _currcanvas = 0;
    canvas[0].setTextColor(foregnd_colour, backgnd_colour);
    cboxofs = canvas[0].width() - (fw - fh + 5);

    drawCheckbox(cboxofs, 0, fh - 1, myProbe, prst);

    for (int i = 0; i < my_n_axis; i++) {
        drawStr( 0, fh + linespc + i * (fh+linespc), DRO_format(i, myAxes[i]).c_str());
        drawCheckbox(cboxofs, fh + linespc + i * (fh+linespc), fh - 1 , myLimits[i], "Limit");
    }
    uint8_t *  buffers [] = {canvas[0].getBuffer(), canvas[1].getBuffer()};
    tft.drawBitmaps(0,0, 2, buffers, canvas[0].width(), canvas[0].height(), _palette, backgnd_colour); 
}

void TFTSPIFastDisplayFunctions::showFileDisplay() {
    canvas[_currcanvas].fillScreen(backgnd_colour);
    //auto fh = tft.getMaxCharHeight();
    int16_t x1, y1;
    uint16_t fh,fw;
    canvas[0].getTextBounds(fname.c_str(),0, 0,&x1, &y1, &fw, &fh);
    drawStr(0, 0, fname.c_str());
    drawProgressBar(20, (fh + linespc) * 2, canvas[0].width() - 20, -fh, f_percent);
    tft.drawBitmap(0,0, canvas[_currcanvas].getBuffer(), canvas[_currcanvas].width(), canvas[_currcanvas].height(), _palette[_currcanvas], backgnd_colour);
}

void TFTSPIFastDisplayFunctions::showRadioDisplay() {
    canvas[_currcanvas].fillScreen(0);
    int16_t x1, y1;
    uint16_t fh,fw;
    canvas[_currcanvas].getTextBounds(_radio_info.c_str(),0, 0,&x1, &y1, &fw, &fh);
    drawStr(0, 0, _radio_info.c_str());
    drawStr(0, (fh + linespc) * 2, _radio_addr.c_str());
    tft.drawBitmap(0,0, canvas[_currcanvas].getBuffer(), canvas[_currcanvas].width(), canvas[_currcanvas].height(), _palette[_currcanvas], backgnd_colour);
}

void TFTSPIFastDisplayFunctions::showWebUIDisplay() {
    canvas[_currcanvas].fillScreen(backgnd_colour);
    int16_t x1, y1;
    uint16_t fh,fw;
    const char *msg =  "WebUI from";
    canvas[_currcanvas].getTextBounds(msg,0, 0,&x1, &y1, &fw, &fh);

    drawStr(0, 0, msg);
    drawStr(0, (fh + linespc) * 2, _webui_addr.c_str());
    tft.drawBitmap(0,0, canvas[_currcanvas].getBuffer(), canvas[_currcanvas].width(), canvas[_currcanvas].height(), _palette[_currcanvas], backgnd_colour);
}
