#include "displayfuncs.h"
#include "U8g2lib.h"
#include "AceButton.h"
#if  defined(HAVE_WS2812_BACKLIGHT) 
#include <FastLED.h>

#define NUM_LEDS    3
#define BRIGHTNESS  64
#define LED_TYPE    WS2812
#define COLOR_ORDER RGB
#endif
#if defined(HAVE_ENCODER)
#include <ESP32RotaryEncoder.h>
#endif

class U8G2DisplayHelpers : public DisplayHelpers {
public:
    U8G2DisplayHelpers(std::shared_ptr<U8G2>display) : u8g2{display} {};
    void drawProgressBar(int x, int y, int width, int height, int percent) override;
    void drawCheckbox(int x, int y, int width, bool checked, std::string label) override;
protected:
    std::shared_ptr<U8G2>u8g2;
};

class U8G2DisplayScreen : public DisplayScreen, public U8G2DisplayHelpers {
public:
    U8G2DisplayScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<U8G2>display) : U8G2DisplayHelpers{display}, DisplayScreen{pb} {};
protected:
    uint16_t linespc = 1; 
};


class U8G2SplashScreen : public U8G2DisplayScreen {
public:
    U8G2SplashScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<U8G2>display) : U8G2DisplayScreen{pb, display} {};
    bool show() override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class U8G2FileScreen : public U8G2DisplayScreen {
public:
    U8G2FileScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<U8G2>display) : U8G2DisplayScreen{pb, display} {};

    bool show() override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class U8G2RadioScreen: public U8G2DisplayScreen {
public:
    U8G2RadioScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<U8G2>display) : U8G2DisplayScreen{pb, display} {};
    bool show()  override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class U8G2DROScreen: public U8G2DisplayScreen {
public:
    U8G2DROScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<U8G2>display) : U8G2DisplayScreen{pb, display} {};
    bool show() override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class U8G2WebUIScreen: public U8G2DisplayScreen {
public:
    U8G2WebUIScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<U8G2>display) : U8G2DisplayScreen{pb, display} {};
    bool show() override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class U8G2DisplayFunctions :  public DisplayFunctions {
public:
#if !defined (HAVE_MINI12864)
    U8G2DisplayFunctions(std::shared_ptr<U8G2> u8g2, std::shared_ptr<DataBag> pd);
    U8G2DisplayFunctions(int rst=5, int scl=SCL, int sda=SDA);
#else
    U8G2DisplayFunctions(std::shared_ptr<U8G2> u8g2, std::shared_ptr<DataBag> pd);
    U8G2DisplayFunctions(int rst=5, int cs=SS, int dc=-1);
#endif
    void begin() override;
    int getNumButtons() { return 3; }
    int getPressedButton();
    int getEncoder();
private: 

    std::shared_ptr<U8G2> u8g2;
    ace_button::AceButton buttons[2];
    class   : public ace_button::IEventHandler {
        bool button_pressed = false;
        void handleEvent(ace_button::AceButton *btn, uint8_t eventType, uint8_t buttonState)  {
            switch (eventType) {
                case ace_button::AceButton::kEventPressed:
                    button_pressed = true;
                break;
                case ace_button::AceButton::kEventReleased:
                    button_pressed = false;
                break;
            }
        }; 
    public:
        bool getPressed() { return button_pressed; }
    } buttonhandler[2];
#if defined(HAVE_WS2812_BACKLIGHT)
    CRGB leds[NUM_LEDS];
#endif
#if defined(HAVE_ENCODER)
    RotaryEncoder rotaryEncoder;
    //int old_encval = -1;
#endif
};