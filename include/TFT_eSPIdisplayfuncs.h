#include "displayfuncs.h"

#include "SPI.h"
#include "TFT_eSPI.h"


#define KEY_W 80 // Width and height
#define KEY_H 80
#define KEY_SPACING_X 120 // X and Y gap
#define KEY_SPACING_Y 3
#define KEY_TEXTSIZE 1   // Font size multiplier
#define NUM_BUTTONS 2

#define PALETTE_BLACK    0  //^
#define PALETTE_BROWN    1  //|
#define PALETTE_RED      2  //|
#define PALETTE_ORANGE   3  //|
#define PALETTE_YELLOW   4  //Colours 0-9 follow the resistor colour code!
#define PALETTE_GREEN    5  //|
#define PALETTE_BLUE     6  //|
#define PALETTE_PURPLE   7  //|
#define PALETTE_DARKGREY 8  //|
#define PALETTE_WHITE    9  //v
#define PALETTE_CYAN     10  //Blue+green mix
#define PALETTE_MAGENTA  11  //Blue+red mix
#define PALETTE_MAROON   12  //Darker red colour
#define PALETTE_DARKGREEN 13 // Darker green colour
#define PALETTE_NAVY     14  //Darker blue colour
#define PALETTE_PINK     15 //




class TFT_eSPIDisplayHelpers : public DisplayHelpers {
public:
    TFT_eSPIDisplayHelpers(TFT_eSPIDisplayHelpers &h) : tft{h.tft}, sprite {h.sprite} {};
    TFT_eSPIDisplayHelpers(std::shared_ptr<TFT_eSPI> display, std::shared_ptr<TFT_eSprite> s) : tft{display}, sprite{s} {};
    void drawStr(int x, int y, const char * str);
    void drawProgressBar(int x, int y, int width, int height, int percent) override;
    void drawCheckbox(int x, int y, int width, bool checked, std::string label) override;
    void drawButtons(uint16_t x, uint16_t y);
    uint16_t getForeground() {return foregnd_colour;};
    uint16_t getBackground() {return backgnd_colour; };
    int getPressedButton();
protected:
    std::shared_ptr<TFT_eSPI> tft;
    std::shared_ptr<TFT_eSprite> sprite;
    TFT_eSPI_Button buttons[NUM_BUTTONS];
    const char * buttonLabels[NUM_BUTTONS] = {"Hold",  "Run"};
    const uint16_t buttonColors[NUM_BUTTONS] = {PALETTE_RED, PALETTE_GREEN};
    uint16_t backgnd_colour = PALETTE_BLACK;
    uint16_t foregnd_colour = PALETTE_WHITE;
    uint16_t linespc = 3; 
    int nButtonsX = NUM_BUTTONS;
    int nButtonsY = 1;
};


class TFT_eSPIDisplayScreen : public DisplayScreen, public TFT_eSPIDisplayHelpers {
public:
    TFT_eSPIDisplayScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<TFT_eSPI> display, std::shared_ptr<TFT_eSprite> sprite) : 
        TFT_eSPIDisplayHelpers{display, sprite}, DisplayScreen{pb}, tft{display}, sprite{sprite} {};

protected:
    std::shared_ptr<TFT_eSPI> tft;;
    std::shared_ptr<TFT_eSprite>sprite;
};


class TFT_eSPISplashScreen : public TFT_eSPIDisplayScreen {
public:
    TFT_eSPISplashScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<TFT_eSPI> display, std::shared_ptr<TFT_eSprite>sprite) : TFT_eSPIDisplayScreen{pb, display, sprite} {};
    bool show() override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;

};

class TFT_eSPIFileScreen : public TFT_eSPIDisplayScreen {
public:
    TFT_eSPIFileScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<TFT_eSPI> display, std::shared_ptr<TFT_eSprite> sprite) : TFT_eSPIDisplayScreen{pb, display, sprite} {};

    bool show() override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class TFT_eSPIRadioScreen: public TFT_eSPIDisplayScreen {
public:
    TFT_eSPIRadioScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<TFT_eSPI> display, std::shared_ptr<TFT_eSprite>sprite) : TFT_eSPIDisplayScreen{pb, display, sprite} {};
    bool show()  override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class TFT_eSPIDROScreen: public TFT_eSPIDisplayScreen {
public:
    TFT_eSPIDROScreen (std::weak_ptr<DataBag> pb, std::shared_ptr<TFT_eSPI> display, std::shared_ptr<TFT_eSprite>sprite) : TFT_eSPIDisplayScreen{pb, display, sprite} {};
    bool show() override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class TFT_eSPIWebUIScreen: public TFT_eSPIDisplayScreen {
public:
    TFT_eSPIWebUIScreen(std::weak_ptr<DataBag> pb, std::shared_ptr<TFT_eSPI> display,std::shared_ptr<TFT_eSprite>sprite) : TFT_eSPIDisplayScreen{pb, display, sprite} {};
    bool show() override;
    bool isSatisfied() override;
    bool needsWait() override;
    void unSatisfy() override;
};

class TFT_eSPIDisplayFunctions :  public DisplayFunctions {
public:
    TFT_eSPIDisplayFunctions();
    TFT_eSPIDisplayFunctions(std::shared_ptr<TFT_eSPI> tft, std::shared_ptr<DataBag> pd);
    TFT_eSPIDisplayFunctions(std::shared_ptr<TFT_eSPI> tft,  std::shared_ptr<TFT_eSprite>spreite, std::shared_ptr<DataBag> pd);
private: 
    std::shared_ptr<TFT_eSPI> tft;
    std::shared_ptr<TFT_eSprite> sprite;
   
public:
    void begin() override;
    /*bool updateDisplay() override;
    bool showFileDisplay() override;
    bool showRadioDisplay() override;
    bool showBootLogo() override;
    bool showWebUIDisplay() override; */
    int getNumButtons() {return NUM_BUTTONS;};
    int getPressedButton();
    void touch_calibrate();
protected:
    TFT_eSPIDisplayHelpers h;
    //TFT_eSPIDisplayScreen &currentScreen;

 //   void drawFrame(int x, int y, int width, int height);       
};