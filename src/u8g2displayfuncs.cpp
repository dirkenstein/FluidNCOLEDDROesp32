#include "u8g2displayfuncs.h"
#include "fncmono.h"
#include "pin_config.h"
#include <sstream>

#if !defined (HAVE_MINI12864)
    U8G2DisplayFunctions::U8G2DisplayFunctions(int rst, int scl, int sda) :
            U8G2DisplayFunctions ( std::make_shared<U8G2_SSD1309_128X64_NONAME2_F_HW_I2C>(U8G2_R0, rst, scl, sda)
            ,std::make_shared<DataBag>()) {};
    U8G2DisplayFunctions::U8G2DisplayFunctions(std::shared_ptr<U8G2>u8g2a, std::shared_ptr<DataBag> pd) :  
        u8g2{u8g2a}
        //, buttons{ace_button::AceButton(PIN_BUTTON_1), ace_button::AceButton(PIN_BUTTON_2)} 
        , DisplayFunctions{
            pd
            , { 
              {SplashScreen, std::make_shared<U8G2SplashScreen>(pd, u8g2a) }
            , {RadioScreen, std::make_shared<U8G2RadioScreen>(pd, u8g2a)} 
            , {DROScreen, std::make_shared<U8G2DROScreen>(pd, u8g2a)}
            , {FileScreen, std::make_shared<U8G2FileScreen>(pd, u8g2a)} 
            , {WebUIScreen, std::make_shared<U8G2WebUIScreen>(pd, u8g2a)} 
          }
         }  
    {
       data->clearAllData();
       clearSatisfaction();
    };

#else
   U8G2DisplayFunctions::U8G2DisplayFunctions(int rst, int cs, int dc) :
            U8G2DisplayFunctions ( std::make_shared<U8G2_ST7565_JLX12864_F_4W_HW_SPI>(U8G2_R2, cs,  dc, rst)
            ,std::make_shared<DataBag>() ) {};

   U8G2DisplayFunctions::U8G2DisplayFunctions(std::shared_ptr<U8G2>u8g2a, std::shared_ptr<DataBag> pd) :  
        u8g2{u8g2a}
        , rotaryEncoder{PIN_ENCODER_A, PIN_ENCODER_B, PIN_ENCODER_SW}   
        , DisplayFunctions{
            pd
            , { 
              {SplashScreen, std::make_shared<U8G2SplashScreen>(pd, u8g2a) }
            , {RadioScreen, std::make_shared<U8G2RadioScreen>(pd, u8g2a)} 
            , {DROScreen, std::make_shared<U8G2DROScreen>(pd, u8g2a)}
            , {FileScreen, std::make_shared<U8G2FileScreen>(pd, u8g2a)} 
            , {WebUIScreen, std::make_shared<U8G2WebUIScreen>(pd, u8g2a)} 
          }
         }  
    {
       data->clearAllData();
    };
#endif

void U8G2DisplayFunctions::begin()  { 
        //U8G2SplashScreen splash(data, u8g2);
        //screens.push_back(splash);
        pinMode (PIN_BUTTON_1, INPUT);
        pinMode(PIN_BUTTON_2, INPUT);
        buttons[1].init(PIN_BUTTON_1);
        ace_button::ButtonConfig* buttonConfig = buttons[0].getButtonConfig();
        buttonConfig->setIEventHandler( &buttonhandler[0]);
        buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureClick);
        buttons[1].init(PIN_BUTTON_2);
        ace_button::ButtonConfig* buttonConfig2 = buttons[1].getButtonConfig();
        buttonConfig2->setIEventHandler( &buttonhandler[1]);
        buttonConfig2->setFeature(ace_button::ButtonConfig::kFeatureClick);
        //buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureClick);
        //buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureDoubleClick);
        //buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureLongPress);
        //buttonConfig->setFeature(ace_button::ButtonConfig::kFeatureRepeatPress);
#if defined(HAVE_WS2812_BACKLIGHT)
        FastLED.addLeds<LED_TYPE, WS2812_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
        FastLED.setBrightness(  255 );
        //fill_solid(leds, NUM_LEDS /*number of leds*/, CRGB::White);        
        leds[0] = CRGB::DarkGoldenrod;
        leds[1] = CRGB::DarkGray;
        leds[2] = CRGB::DarkGray;
        FastLED.show();
#endif
#if defined(HAVE_MINI12864)
        pinMode (PIN_ENCODER_A, INPUT);
        pinMode (PIN_ENCODER_B, INPUT);
	    rotaryEncoder.setEncoderType( EncoderType::FLOATING );
        rotaryEncoder.setBoundaries( 1, 10, true );
	    //rotaryEncoder.onTurned( &knobCallback );
        //rotaryEncoder.onPressed( &buttonCallback );
        rotaryEncoder.begin();
#endif
        u8g2->begin();
        //u8g2->setRotation(3);

        u8g2->clearDisplay();
        u8g2->clearBuffer();

        u8g2->setFont(u8g2_font_6x10_tf);
        u8g2->setFontRefHeightExtendedText();
        u8g2->setFontMode(0);
        u8g2->setFontPosTop(); 
        u8g2->setFontDirection(0);
        u8g2->setDrawColor(1);
        u8g2->drawBox(0, 0, DISP_WIDTH, DISP_HEIGHT);
        u8g2->drawStr(0, 0, "FluidNC Channel pendant");
        //std::string out;
        //std::stringstream ppp;

        //for (auto it = screens.cbegin(); it != screens.cend(); ++it) {
        //        it->second->setBag(datap);
        //    ppp << " Screen: " << it->first << it->second->data.lock() << "\n"; 
        //}
        //out = ppp.str();
        //debug_println(out.c_str());

    };





void U8G2DisplayHelpers::drawProgressBar(int x, int y, int width, int height, int percent) {
    u8g2->drawFrame(x, y, width, height);
    u8g2->drawBox(x, y, (uint32_t)(width*100)/percent, height); 
}

void U8G2DisplayHelpers::drawCheckbox(int x, int y, int width, bool checked, std::string label) {
    if (checked) {
        u8g2->drawBox(x, y, width, width);
    } else {
        u8g2->drawFrame(x, y, width, width);
    }
    u8g2->drawStr(x + width + 5, y, label.c_str());
}

bool U8G2SplashScreen::show() {
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    u8g2->clearBuffer();
    u8g2->setDrawColor(1);
    u8g2->drawXBMP(-1, (u8g2->getDisplayHeight()-logo_height)/2, logo_width, logo_height, logo_bits);
    int fh = u8g2->getMaxCharHeight();
    std::string verbuf = "";
    bool got_grbl = false;
    bool got_fluidnc = false;
    if (datap->getVersion().length() > 0) {
      verbuf += datap->getVersion();
      got_fluidnc = true;
    }
    if (datap->getGrblVersion().length() > 0) {
      verbuf += " Grbl " + datap->getGrblVersion();
      got_grbl = true;
    }
    u8g2->drawStr(0, u8g2->getDisplayHeight() - fh, verbuf.c_str());
    u8g2->sendBuffer();
    satisfied =  got_grbl && got_fluidnc;
    return satisfied;
}

bool U8G2SplashScreen::isSatisfied () { return satisfied; }
bool U8G2SplashScreen::needsWait () { return true; }
void U8G2SplashScreen::unSatisfy() { satisfied = false; }

bool U8G2DROScreen::show() {
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    char   buf[12];
    const int cboxofs = 80;
    std::string axesNames = "XYZABC";
    u8g2->clearBuffer();
    if (datap->getMyState() == "Alarm") {
        u8g2->setDrawColor(0);
    } else if (datap->getMyState().find("Hold") == 0) {
         u8g2->setDrawColor(0);
    } else {
        u8g2->setDrawColor(1);
    }
    int  lnHeight = u8g2->getMaxCharHeight();
    int stateW = u8g2->getStrWidth(datap->getMyState().c_str());
    u8g2->drawStr( 0, 0, datap->getMyState().c_str());
    u8g2->setDrawColor(1);
    char mistFloodBuf[20];
    char spindleBuf[10];
    snprintf(spindleBuf, 10, "S:%d ", datap->getSpindleNum());
    snprintf(mistFloodBuf, 20, " %s%s%s", datap->getSpindleNum() > 0 ? spindleBuf: "", datap->isMist() ? "Mi": "", datap->isFlood() ? "Fl" : "");
    u8g2->drawStr( stateW, 0, mistFloodBuf);
    drawCheckbox(cboxofs, 0, lnHeight - 1, datap->isProbe(), "Probe");
    for (int i = 0; i < datap->getNAxes(); i++) {
        u8g2->drawStr( 0, lnHeight + i * lnHeight, datap->DRO_format(i, datap->getAxis(i)).c_str());
        drawCheckbox(cboxofs, lnHeight + i * lnHeight, lnHeight - 1 , datap->isLimit(i), "Limit");
    }

    u8g2->sendBuffer();
    satisfied = true;
    return satisfied;
}


bool U8G2DROScreen::isSatisfied () { return satisfied; }
bool U8G2DROScreen::needsWait () { return false; }
void U8G2DROScreen::unSatisfy() { /* do nothing */}

bool U8G2FileScreen::show() {
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }    
    u8g2->clearBuffer();
    auto fh = u8g2->getMaxCharHeight();
    u8g2->drawStr(0, 0, datap->getFname().c_str());
    drawProgressBar(10, (fh + linespc) * 2, u8g2->getDisplayWidth() - 20, 12, datap->getPercent());
    u8g2->sendBuffer();
    satisfied =  datap->getPercent() >= 100;
    return satisfied;
}
bool U8G2FileScreen::isSatisfied () { return satisfied; }
bool U8G2FileScreen::needsWait () { return true; }
void U8G2FileScreen::unSatisfy() { satisfied = false;}

bool U8G2RadioScreen::show() {
    auto datap = data.lock();
        if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    u8g2->clearBuffer();
    auto fh = u8g2->getMaxCharHeight();
    std::string sep = "";
    if (datap->getRadioType().length() > 0) {
      sep = ": ";
    }
    u8g2->drawStr(0, 0, (datap->getRadioType() + sep + datap->getRadioInfo()).c_str());
    u8g2->drawStr(0, (fh + linespc) * 2, datap->getRadioAddr().c_str());
    u8g2->sendBuffer();
    satisfied = datap->getRadioInfo().length() >0 && datap->getRadioAddr().length() > 0;
    return satisfied;
}
bool U8G2RadioScreen::isSatisfied () { return satisfied; }
bool U8G2RadioScreen::needsWait () { return true; }
void U8G2RadioScreen::unSatisfy() { satisfied = false;}

bool U8G2WebUIScreen::show() {
    auto datap = data.lock();
    if (!datap) {
        debug_println("AAgh no pointer");
        return false;
    }
    u8g2->clearBuffer();
    auto fh = u8g2->getMaxCharHeight();
    u8g2->drawStr(0, 0, "WebUI from");
    u8g2->drawStr(0, (fh + linespc) * 2, datap->getWebUIAddr().c_str());
    u8g2->sendBuffer();
    satisfied = true;
    return satisfied;
}

bool U8G2WebUIScreen::isSatisfied () { return satisfied; }
bool U8G2WebUIScreen::needsWait () { return true; }
void U8G2WebUIScreen::unSatisfy() { satisfied = false;}

int U8G2DisplayFunctions::getPressedButton() {
    buttons[0].check();
    buttons[1].check();
#if defined (HAVE_MINI12864)
    bool rpressed = rotaryEncoder.buttonPressed();
#endif
    bool b1p = buttonhandler[0].getPressed();
    bool b2p = buttonhandler[1].getPressed();
    int bpressed = -1;
    if (b1p) bpressed = 0;
    if (b2p) bpressed = 1;
#if defined (HAVE_MINI12864)
    if (rpressed) bpressed = 2;
#endif
    return  bpressed;
}

int U8G2DisplayFunctions::getEncoder() {
#if defined (HAVE_MINI12864)
   bool rturned = rotaryEncoder.encoderChanged();
    if (rturned) return rotaryEncoder.getEncoderValue();
    else return -1;
#else
    return -1;
#endif
}

