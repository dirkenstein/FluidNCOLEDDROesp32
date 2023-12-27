#pragma once
#include <string>
#include "GrblParserC.h"
#include "Arduino.h"
#include <map>
#include <memory>

enum class ScreenType {
    SplashScreen = 0,
    RadioScreen,
    DROScreen,
    FileScreen,
    WebUIScreen,
    NUM_SCREENS
} ;

class DataBag {
    public:
        DataBag () : startupState{"No Data..."}, myState{startupState}, my_n_axis{4}, myAxes{0 }, myLimits{false }  { clearAllData();}
        void setMyState(const std::string &state) { if (myState != state)  dro_changed = true; myState = state; }
        std::string getMyState() { return myState;}
        void setStartupState() { if (myState != startupState)  dro_changed = true; myState = startupState;  }
        std::string getStartupState() { return startupState;}

        bool isRunState() { return myState == "Run"; }
        bool isHoldState() { return myState.find("Hold") == 0; }

        void setFname(std::string name) {if (fname != name) file_changed = true; name = fname;  }
        std::string getFname() { return fname;}

        void setAxis(int axis, pos_t val) { if (myAxes[axis] != val)  dro_changed = true; myAxes[axis] = val; }
        pos_t getAxis(int axis) { return myAxes[axis]; }
        void setLimit(int axis, bool val) {if (myLimits[axis] != val)  dro_changed = true; myLimits[axis] = val;}
        bool isLimit(int axis) { return myLimits[axis];}
        void setNAxes(int n_axes) { if (my_n_axis != n_axes)  dro_changed = true; my_n_axis = n_axes; }
        int getNAxes() { return my_n_axis; }
        void setProbe (bool probe) { if (probe != myProbe)  dro_changed = true; myProbe = probe;}
        bool isProbe () { return myProbe; }

        void setMM (bool mm) {if (use_mm != mm)  dro_changed = true; use_mm = mm;}
        bool isMM () {return use_mm; }
        void setPercent(uint32_t percent) {if (f_percent != percent)  file_changed = true; f_percent = percent; }
        uint32_t  getPercent () {return f_percent; }
        void setMist (bool mist) {if(s_mist != mist) dro_changed = true; s_mist = mist;}
        bool isMist() { return s_mist;}
        void setFlood (bool flood) {if(s_flood != flood) dro_changed = true; s_flood = flood;}
        bool isFlood() { return s_flood;}

        void setSpindleNum(int n) { if (spindleNum != n) dro_changed = true; spindleNum = n; };
        int getSpindleNum () { return spindleNum; }
        void setRadioType(std::string type) { if (type != _radio_type) radio_changed = true; _radio_type = type; }
        std::string  getRadioType() {  return _radio_type; }
        void setRadioInfo(std::string info) {  if (info != _radio_info) radio_changed = true;_radio_info = info; }
        std::string  getRadioInfo() { return _radio_info; }

        void setRadioAddr(std::string addr) { if (addr != _radio_addr) radio_changed = true; _radio_addr = addr; }
        std::string getRadioAddr() { return _radio_addr; }

        void setVersion(std::string version) {if (version != fluidnc_version) version_changed = true;fluidnc_version = version;}
        std::string getVersion() { return fluidnc_version; }
        void setGrblVersion(std::string version) {if (version != grbl_version) version_changed = true; grbl_version = version;}
        std::string getGrblVersion() { return grbl_version; }
        void setWebUIAddr(std::string addr) {if (_webui_addr != addr) _webui_changed = true; _webui_addr = addr;}
        std::string getWebUIAddr() { return _webui_addr; }
        bool isDROChanged() { return dro_changed;}
        bool isVersionChanged() { return version_changed;}
        bool isFileChanged() { return file_changed;}
        bool isWebUICanged() { return _webui_changed = false; }
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
            dro_changed = false;
            radio_changed = false;
            version_changed =false;
            _webui_changed = false;
            file_changed = false;
        }
        std::string DRO_format(int axis, pos_t val);
    protected:
        const std::string startupState;
        std::string myState              = startupState;
        pos_t  myAxes[MAX_N_AXIS]   = { 0 };
        int    my_n_axis            = 4;
        bool   myLimits[MAX_N_AXIS] = { false };
        bool   myProbe              = false;
        bool   use_mm               = true;
        bool   s_mist               = false;
        bool   s_flood              = false;
        int    spindleNum           = -1;
        bool dro_changed = false;

        std::string fname                = "";
        file_percent_t  f_percent   = 0;
        bool file_changed = false;

        std::string _radio_type; 
        std::string _radio_info;
        std::string _radio_addr;
        bool radio_changed          = false;

        std::string _webui_addr;   
        bool _webui_changed          = false;
        std::string fluidnc_version = "";
        std::string grbl_version    = "";
        bool version_changed        = false;
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
        currentScreen{screens[ScreenType::SplashScreen]},  
        currentScreenType{ScreenType::SplashScreen} 
        { 
            for (auto ic = screens.cbegin(); ic != screens.cend(); ++ic) {
                rscreens[ic->second] = ic->first;
            }
            data->clearAllData(); 
            clearSatisfaction();

        };  
    DisplayFunctions() = delete;
    virtual void begin() = 0;
    std::weak_ptr<DisplayScreen> getScreen(ScreenType t) { return screens [t]; }
    std::weak_ptr<DisplayScreen> firstScreen() { return screens[ScreenType::SplashScreen];}
    std::weak_ptr<DisplayScreen> getNextScreen() { return screens[static_cast<ScreenType>(((int)currentScreenType +1) % (int)ScreenType::NUM_SCREENS)];}
    std::weak_ptr<DisplayScreen>  getCurrentScreen() { return currentScreen;}
    void setCurrentScreen(std::weak_ptr<DisplayScreen> p) { currentScreen = p;  currentScreenType = rscreens[p.lock()]; };
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
    std::map <std::shared_ptr<DisplayScreen>, ScreenType> rscreens;

    std::weak_ptr <DisplayScreen> currentScreen;
    ScreenType currentScreenType;
};
