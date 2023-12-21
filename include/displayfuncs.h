#pragma once
#include <string>
#include "GrblParserC.h"
#include "Arduino.h"
#include <map>
#include <memory>

typedef enum screens_e {
    SplashScreen = 0,
    RadioScreen,
    DROScreen,
    FileScreen,
    WebUIScreen,
} ScreenType;

class DataBag {
    public:
        DataBag () : startupState{"No Data..."}, myState{startupState}, my_n_axis{4}, myAxes{0 }, myLimits{false }  { clearAllData();}
        void setMyState(const std::string &state) { myState = state;}
        std::string getMyState() { return myState;}
        void setStartupState() { myState = startupState;}
        std::string getStartupState() { return startupState;}

        bool isRunState() { return myState == "Run"; }
        bool isHoldState() { return myState.find("Hold") == 0; }

        void setFname(std::string name) {name = fname;}
        std::string getFname() { return fname;}
        void setAxis(int axis, pos_t val) { myAxes[axis] = val;}
        pos_t getAxis(int axis) { return myAxes[axis];}
        void setLimit(int axis, bool val) {myLimits[axis] = val;}
        bool isLimit(int axis) { return myLimits[axis];}
        void setNAxes(int n_axes) { my_n_axis = n_axes; }
        int getNAxes() { return my_n_axis; }
        void setProbe (bool probe) {myProbe = probe;}
        bool isProbe () { return myProbe; }

        void setMM (bool mm) {use_mm = mm;}
        bool isMM () {return use_mm; }
        void setPercent(uint32_t percent) {f_percent = percent; }
        uint32_t  getPercent () {return f_percent; }
        void setMist (bool mist) {s_mist = mist;}
        bool isMist() { return s_mist;}
        void setFlood (bool flood) {s_flood = flood;}
        bool isFlood() { return s_mist;}

        void setSpindleNum(int n) { spindleNum = n; };
        int getSpindleNum () { return spindleNum; }
        void setRadioType(std::string type) { _radio_type = type; }
        std::string  getRadioType() { return _radio_type; }
        void setRadioInfo(std::string info) { _radio_info = info; }
        std::string  getRadioInfo() { return _radio_info; }

        void setRadioAddr(std::string addr) { _radio_addr = addr; }
        std::string getRadioAddr() { return _radio_addr; }

        void setVersion(std::string version) {fluidnc_version = version;}
        std::string getVersion() { return fluidnc_version; }
        void setGrblVersion(std::string version) {grbl_version = version;}
        std::string getGrblVersion() { return grbl_version; }
        void setWebUIAddr(std::string addr) { _webui_addr = addr;}
        std::string getWebUIAddr() { return _webui_addr; }
        void clearAllData() {
            clearData();
            myState = startupState;
            _radio_type = "";
            _radio_info = "";
            _radio_addr = "";
            _webui_addr = "";
            fluidnc_version = "";
            grbl_version = "";
        }
        void clearData() {
            for (int x = 0; x < MAX_N_AXIS; x++) {
                myAxes[x] = 0;
                myLimits[x] = false;
            }
            myProbe = false;
            s_mist = false;
            s_flood = false;
            spindleNum = -1;
            fname    = "";
        }
        std::string DRO_format(int axis, pos_t val);
    protected:
        const std::string startupState;
        std::string myState              = startupState;
        std::string fname                = "";
        pos_t  myAxes[MAX_N_AXIS]   = { 0 };
        int    my_n_axis            = 4;
        bool   myLimits[MAX_N_AXIS] = { false };
        bool   myProbe              = false;
        bool   use_mm               = true;
        file_percent_t  f_percent   = 0;
        bool   s_mist               = false;
        bool   s_flood              = false;
        int    spindleNum           = -1;
        std::string _radio_type; 
        std::string _radio_info;
        std::string _radio_addr;
        std::string _webui_addr;   
        std::string fluidnc_version         = "";
        std::string grbl_version         = "";
};

class DisplayHelpers {
protected:
    virtual void drawProgressBar(int x, int y, int width, int height, int percent) = 0;
    virtual void drawCheckbox(int x, int y, int width, bool checked, std::string label) = 0;
    // local copies so we can do one update function
};

class DisplayScreen {
    public:
        DisplayScreen() : satisfied{false} { };
        DisplayScreen(std::weak_ptr<DataBag> pd) : satisfied{false}, data{pd} { };
        void setBag(std::weak_ptr<DataBag> pd) { data = pd; }
        virtual bool show() =0;
        virtual bool isSatisfied() = 0;
        virtual bool needsWait() = 0;
        virtual void unSatisfy() = 0;
    protected: 
        bool satisfied;
        std::weak_ptr <DataBag> data;        
};


class DisplayFunctions {
public:
   DisplayFunctions(std::shared_ptr<DataBag> pd, std::map<ScreenType, std::shared_ptr<DisplayScreen>> screenm) : 
        data{pd},
        screens{screenm}, 
        currentScreen{screens[SplashScreen]},  
        currentScreenType{SplashScreen} 
        { 
            data->clearAllData(); 
            clearSatisfaction();
        };  
    DisplayFunctions() = delete;
    virtual void begin() = 0;
    std::weak_ptr<DisplayScreen> getScreen(ScreenType t) { return screens [t]; }
    std::weak_ptr<DisplayScreen> firstScreen() { return screens[SplashScreen];}
    std::weak_ptr<DisplayScreen>  getCurrentScreen() { return currentScreen;}
    void setCurrentScreen(std::weak_ptr<DisplayScreen> p) { currentScreen = p;};
    int getNumButtons() { return 0; };
    virtual int getPressedButton() {return -1;};
    virtual int getEncoder(){ return -1;};
    std::shared_ptr<DataBag> data;

    void clearSatisfaction() {
        for (auto it = screens.cbegin(); it != screens.cend(); ++it) {
            auto swp = it->second;
            swp->unSatisfy();
        }
        
    }
protected:
    std::map <ScreenType, std::shared_ptr<DisplayScreen>> screens;
    std::weak_ptr <DisplayScreen> currentScreen;
    ScreenType currentScreenType;
};
