#include "TFT_eSPIdisplayfuncs.h"
#include "fnc.h"
#include "pin_config.h"
#include "FS.h"
#include <LittleFS.h>

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData1"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false
TFT_eSPIDisplayFunctions::TFT_eSPIDisplayFunctions() : 
    TFT_eSPIDisplayFunctions(std::make_shared<TFT_eSPI>(),  std::make_shared<DataBag>()) {};

TFT_eSPIDisplayFunctions::TFT_eSPIDisplayFunctions(std::shared_ptr<TFT_eSPI> tfta, std::shared_ptr<DataBag> pd) : 
   TFT_eSPIDisplayFunctions(tfta, std::make_shared<TFT_eSprite>(tfta.get()), pd) {};     

TFT_eSPIDisplayFunctions::TFT_eSPIDisplayFunctions(
      std::shared_ptr<TFT_eSPI> tfta,
      std::shared_ptr<TFT_eSprite>spritea, 
      std::shared_ptr<DataBag> pd) : 
        tft{tfta} 
      , sprite{spritea}
      , DisplayFunctions{
            pd
            , { 
              {ScreenType::SplashScreen, std::make_shared<TFT_eSPISplashScreen> (pd, tfta, spritea) }
            , {ScreenType::RadioScreen, std::make_shared<TFT_eSPIRadioScreen>(pd, tfta, spritea)} 
            , {ScreenType::DROScreen, std::make_shared<TFT_eSPIDROScreen>(pd, tfta, spritea)}
            , {ScreenType::FileScreen, std::make_shared<TFT_eSPIFileScreen>(pd, tfta, spritea)} 
            , {ScreenType::WebUIScreen, std::make_shared<TFT_eSPIWebUIScreen>(pd,tfta, spritea)} 
          }
         }  
         , h{tft, sprite}
{ 
        data->clearAllData();
        clearSatisfaction();
};

void TFT_eSPIDisplayFunctions::begin()  { 
        pinMode (PIN_DISPLAY_RESET, OUTPUT);
        pinMode(PIN_DISPLAY_LCD, OUTPUT);
        ledcSetup(0, 5000, 8);
        ledcAttachPin(PIN_DISPLAY_LCD, 0);
        ledcWrite(0, LED_BRIGHTNESS);
        //touchscreen.begin(PIN_TSC_CS, &myspi, TS_RESISTANCE);
        //touchscreen.enableInterrupts(true);
        //touchscreen.setTouchedThreshold(10000.0f);  

        tft->setSwapBytes(true);   //u8g2.setRotation(3);
        tft->begin();
        //tft->setSPISpeed(40000000L);
        tft->setRotation(3);
        tft->setTextFont(4);
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
        //tft->setFreeFont(&FreeSans18pt7b);
        sprite->setColorDepth(4);
        sprite->createSprite(tft->width(), tft->height());
        sprite->setRotation(0);
        sprite->setTextFont(4);
        //sprite->setFreeFont(&FreeSans18pt7b);

        sprite->setTextWrap(false);
        /* sprite->createPalette((const uint16_t *) nullptr, 16);
        sprite->setPaletteColor(0, backgnd_colour);
        sprite->setPaletteColor(1, foregnd_colour);
        sprite->setPaletteColor(2, TFT_RED);
        sprite->setPaletteColor(3, TFT_YELLOW);
        sprite->setPaletteColor(4, TFT_GREEN); */
        sprite->setTextColor(h.getForeground(), h.getBackground());
        sprite->fillSprite(h.getBackground());
        touch_calibrate();
        tft->setTextFont(4);

}


void TFT_eSPIDisplayFunctions::touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    Serial.println("Formatting file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
      fs::File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft->setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft->fillScreen(TFT_BLACK);
    tft->setCursor(20, 0);
    tft->setTextFont(2);
    tft->setTextSize(1);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);

    tft->println("Touch corners as indicated");

    tft->setTextFont(1);
    tft->println();

    if (REPEAT_CAL) {
      tft->setTextColor(TFT_RED, TFT_BLACK);
      tft->println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft->calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->println("Calibration complete!");

    // store data
    fs::File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}


void TFT_eSPIDisplayHelpers::drawButtons(uint16_t x, uint16_t y) {
  // Generate buttons with different size X deltas
  for (int ix = 0; ix < nButtonsX; ix++)
  {
    for (int iy = 0; iy < nButtonsY; iy++) 
    {
     buttons[ix + nButtonsX*iy].initButton(sprite.get(),
                      (sprite->width() - (2*KEY_W + KEY_SPACING_X))/2 +  KEY_W/2 + ix * (KEY_W + KEY_SPACING_X),
                      y + KEY_H/2 + KEY_SPACING_Y + iy * (KEY_H + KEY_SPACING_Y), // x, y, w, h, outline, fill, text
                      KEY_W,
                      KEY_H,
                      PALETTE_WHITE, // Outline
                      buttonColors[ix + nButtonsX*iy ], // Fill
                      PALETTE_BLACK, // Text
                      "", // 10 Byte Label
                      KEY_TEXTSIZE);


      buttons[ix + nButtonsX*iy].setLabelDatum(0,  0 /* +  KEY_H/2 - sprite->fontHeight()/2 */, MC_DATUM);
      // Draw button and specify label string
      // Specifying label string here will allow more than the default 10 byte label
      buttons[ix + nButtonsX*iy].drawButton(false, buttonLabels[ix + nButtonsX*iy]);
    }
  }
}

int TFT_eSPIDisplayFunctions::getPressedButton() {
    auto sp = currentScreen.lock();
    //return std::dynamic_pointer_cast<TFT_eSPIDisplayScreen>(sp)->getPressedButton();
    return std::static_pointer_cast<TFT_eSPIDisplayScreen>(sp)->getPressedButton();
}

int TFT_eSPIDisplayHelpers::getPressedButton() {

  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

  // Get current touch state and coordinates
  bool pressed = tft->getTouch(&t_x, &t_y);

  // Adjust press state of each key appropriately
  for (uint8_t b = 0; b < NUM_BUTTONS; b++) {
    if (pressed && buttons[b].contains(t_x, t_y)) 
      buttons[b].press(true);  // tell the button it is pressed
    else 
      buttons[b].press(false);  // tell the button it is NOT pressed
  }

  bool anyChange = false;
  int keyPressed = -1;
  // Check if any key has changed state
  for (uint8_t b = 0; b < NUM_BUTTONS; b++) {
    // If button was just pressed, redraw inverted button
    if (buttons[b].justPressed()) {
      //Serial.println("Button " + (String)b + " pressed");
      buttons[b].drawButton(true, buttonLabels[b]);
      anyChange = true;
      keyPressed = b;
    }
    // If button was just released, redraw normal color button
    if (buttons[b].justReleased()) {
      //Serial.println("Button " + (String)b + " released");
      buttons[b].drawButton(false, buttonLabels[b]);
      anyChange = true;
    }
  }
  if (anyChange) {
    sprite->pushSprite(0,0);
  }
  return keyPressed;
}

void TFT_eSPIDisplayHelpers::drawStr(int x, int y, const char * str) {
    uint16_t fh = sprite->fontHeight();
    sprite->setCursor(x, y);
    sprite->print(str);
}

void TFT_eSPIDisplayHelpers::drawProgressBar(int x, int y, int width, int height, int percent) {
    sprite->drawRect(x, y, width, height, foregnd_colour);
    sprite->fillRect(x, y, (uint32_t)((uint32_t)width*((uint32_t)percent*100))/10000, height, foregnd_colour); 
}


void TFT_eSPIDisplayHelpers::drawCheckbox(int x, int y, int width, bool checked, std::string label) {
    if (checked) {
        sprite->fillRect(x, y, width, width, foregnd_colour);
    } else {
        sprite->drawRect(x, y, width, width, foregnd_colour);
    }
    drawStr(x + width + 5, y, label.c_str());
}

bool TFT_eSPISplashScreen::show() {
    tft->fillScreen(backgnd_colour);
    const int logo_height = 170;
    const int logo_width = 320;
    Serial.println("Boot Logo");
    tft->pushImage(0, (tft->height()-logo_height)/2, logo_width, logo_height, logo);
    uint16_t fh = tft->fontHeight();
    std::string verbuf = "";
    bool got_grbl = false;
    bool got_fluidnc = false;
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    datap->clearVersionChange();
    if (datap->getVersion().length() > 0) {
      verbuf += datap->getVersion();
      got_fluidnc = true;
    }
    if (datap->getGrblVersion().length() > 0) {
      verbuf += " Grbl " + datap->getGrblVersion();
      got_grbl = true;
    }
    tft->setCursor(0, tft->height() -fh);
    tft->print(verbuf.c_str());
    satisfied =  got_grbl && got_fluidnc;
    return satisfied;
}

bool TFT_eSPISplashScreen::isSatisfied() { return satisfied; }
bool TFT_eSPISplashScreen::needsWait() { return true; }
void TFT_eSPISplashScreen::unSatisfy() { satisfied = false; }

bool TFT_eSPIDROScreen::show() {
    char   buf[12];
    int cboxofs; 
    std::string axesNames = "XYZABC";
    sprite->fillSprite(backgnd_colour);
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    datap->clearDROChange();
    if (datap->getMyState() == "Alarm") {
        sprite->setTextColor(PALETTE_RED, backgnd_colour);
    } else if (datap->getMyState().find("Hold") == 0) {
        sprite->setTextColor(PALETTE_YELLOW, backgnd_colour);
    } else {
        sprite->setTextColor(PALETTE_GREEN, backgnd_colour);
    }
    const char * prst = "Probe"; 
    uint16_t fh = sprite->fontHeight();
    uint16_t prw = sprite->textWidth(prst);
    //Serial.printf("X1: %d Y1: %d W:%d H:%d", x1, y1, fw, fh);
    uint16_t stateW = sprite->textWidth(datap->getMyState().c_str());
    drawStr(0, 0, datap->getMyState().c_str());
    sprite->setTextColor(foregnd_colour, backgnd_colour);
    char mistFloodBuf[20];
    char spindleBuf[10];
    snprintf(spindleBuf, 10, "S:%d ", datap->getSpindleNum());
    snprintf(mistFloodBuf, 20, " %s%s%s", datap->getSpindleNum() > 0 ? spindleBuf: "", datap->isMist() ? "Mi": "", datap->isFlood() ? "Fl" : "");
    drawStr( stateW, 0, mistFloodBuf);
    cboxofs = sprite->width() - (prw + fh + 5);

    drawCheckbox(cboxofs, 0, fh - 1, datap->isProbe(), prst);

    for (int i = 0; i < datap->getNAxes(); i++) {
        drawStr( 0, fh + linespc + i * (fh+linespc), datap->DRO_format(i, datap->getAxis(i)).c_str());
        drawCheckbox(cboxofs, fh + linespc + i * (fh+linespc), fh - 1 , datap->isLimit(i), "Limit");
    }
    uint16_t endy = sprite->getCursorY();
    drawButtons(0, endy + fh + linespc);
    sprite->pushSprite(0,0);
    satisfied = true;
    return satisfied;
}


bool TFT_eSPIDROScreen::isSatisfied() { return satisfied; }
bool TFT_eSPIDROScreen::needsWait() { return false; }
void TFT_eSPIDROScreen::unSatisfy() { /* Do nothing #*/ }

bool TFT_eSPIFileScreen::show() {
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    datap->clearFileChange();
    sprite->fillSprite(backgnd_colour);
    uint16_t fh = sprite->fontHeight();
    const char * title = "File:";
    auto tw = sprite->textWidth(title);
    drawStr((sprite->width() -tw)/2,0,title);
    drawStr(0, (fh + linespc), datap->getFname().c_str());
    drawProgressBar(20, (fh + linespc) * 3, sprite->width() - 20, -fh, datap->getPercent());
    sprite->pushSprite(0,0);
    satisfied = datap->getPercent() >= 100;
    return satisfied;
}

bool TFT_eSPIFileScreen::isSatisfied() { return satisfied; }
bool TFT_eSPIFileScreen::needsWait() { return true; }
void TFT_eSPIFileScreen::unSatisfy() { satisfied = false; }

bool TFT_eSPIRadioScreen::show() {
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    datap->clearRadioChange();
    sprite->fillScreen(0);
    uint16_t fh;
    fh = sprite->fontHeight();
    std::string sep = "";
    if (datap->getRadioType().length() > 0) {
      sep = ": ";
    }
    const char * title = datap->getRadioType() == "BT" ? "Bluetooth:" : "WiFi";
    auto tw =  sprite->textWidth(title);
    drawStr((sprite->width() -tw)/2,0,title);
    drawStr(0, 0, (datap->getRadioType() + sep + datap->getRadioInfo()).c_str());
    drawStr(0, (fh + linespc) * 2, datap->getRadioAddr().c_str());
    sprite->pushSprite(0,0);
    satisfied = datap->getRadioInfo().length() >0 && datap->getRadioAddr().length() > 0;
    return satisfied;
}

bool TFT_eSPIRadioScreen::isSatisfied() { return satisfied; }
bool TFT_eSPIRadioScreen::needsWait() { return true; }
void TFT_eSPIRadioScreen::unSatisfy() { satisfied = false; }

bool TFT_eSPIWebUIScreen::show() {
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    datap->clearWebUIChange();
    sprite->fillSprite(backgnd_colour);
    uint16_t fh;
    const char *msg =  "WebUI from";
    fh = sprite->fontHeight();
    drawStr(0, 0, msg);
    drawStr(0, (fh + linespc) * 2, datap->getWebUIAddr().c_str());
    sprite->pushSprite(0,0);
    satisfied =  true;
    return satisfied;
}


bool TFT_eSPIWebUIScreen::isSatisfied() { return satisfied; }
bool TFT_eSPIWebUIScreen::needsWait() { return true; }
void TFT_eSPIWebUIScreen::unSatisfy() { satisfied = false; }
