/*
 * 
 * 
 *  Note: If the USB is not connected this program locks up when that serial port is used. 
 *        Maybe there are some floating pins that generate data.
 */

#include "Arduino.h"
#include "pgmspace.h"

#include "pin_config.h"
#include "GrblParserC.h"  // be sure to move the files into the library folder for Arduino
#include <string>
#include "u8g2displayfuncs.h"
#include "TFT_eSPIdisplayfuncs.h"
#include <list>

//U8G2_ST7565_KS0713_1_4W_HW_SPI U8G2_R0, /* reset=*/ PIN_DISPLAY_RESET, /* clock=*/ SCL, /* data=*/ SDA
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#    error "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif

#define FNCSerial Serial1
#define DebugSerial Serial


#define ECHO_RX_DATA
#define DEBUG_USB

#if defined(HAVE_TFT)
TFT_eSPIDisplayFunctions display;
#else
#if !defined(HAVE_MINI12864)
U8G2DisplayFunctions display (PIN_DISPLAY_RESET, /* clock=*/ SCL, /* data=*/ SDA);
#else
U8G2DisplayFunctions display (PIN_DISPLAY_RESET, /* cs=*/ PIN_DISPLAY_CS, /* dc=*/ PIN_DISPLAY_DC);
#endif
#endif

void parse_AP(std::string &data);
void parse_STA(std::string &data);
void parse_IP(std::string &data);
void parse_Mode(std::string &data);

void parse_BT(std::string &data);
void parse_WebUI(std::string &data);
void parse_VER(std::string &data);
void do_Restart();



uint32_t radio_delay       = 1000;
uint32_t last_report = 0;
uint32_t max_report_interval = 2000;
uint32_t request_retry_interval = 1000;
bool initial_msg_loop = true;
uint32_t satisfaction_delay = 3000;
uint32_t initial_satisfaction_delay = 10000;

uint32_t display_delay = 0;
uint32_t last_delay_start = 0;
uint32_t satisfaction_start = 0;
bool    delaying   = false;
bool    satisfaction_delaying = false;

//typedef bool (DisplayFunctions::* dispfun)(void); 
std::list<std::weak_ptr<DisplayScreen>> toBeDisplayed;
std::weak_ptr<DisplayScreen> currently_displaying = display.firstScreen();

void set_display_delay(uint32_t ms, bool satisfied) {
    display_delay = ms;
    last_delay_start = millis();
    if(!satisfied) {
        satisfaction_start = millis();
        satisfaction_delaying =  true;
    } else {
        satisfaction_start = 0;
        satisfaction_delaying =  false;
    }
    delaying = true;
}

void nonrepeating_screen_delay(ScreenType t, bool compel) {
    auto d = display.getScreen(t);
    auto dp = d.lock();
    if (!dp) {
        debug_println("Aaagh, no pointer ...");
        return;
    }
    //Show the page if there is no delay or if the delay is for the same page
    auto cs = display.getCurrentScreen().lock();
    if (!cs){
        debug_println("Cannot get current screen");
        return;
    }
    //Don't show a delaying screen again automatically if it showed all its data last time
    if(dp->needsWait() && dp->isSatisfied() && !compel) return;
    if (!delaying || (delaying && cs == dp )) {
        bool satisfied = dp->show();
        //currently_displaying = d;
        display.setCurrentScreen(d);
        if (dp->needsWait()) set_display_delay(radio_delay, satisfied);
    } else {
          //Add the page to the end of the list if it's not already there

          if (toBeDisplayed.empty() || toBeDisplayed.back().lock() != dp) 
            toBeDisplayed.push_back(d);
    }
}

extern "C" void show_state(const char* state) {
    display.data->setMyState(state);
}

extern "C" void show_dro(const pos_t* axes, const pos_t* wcos, bool isMpos, bool* limits, size_t n_axis) {
    display.data->setNAxes(n_axis);
    for (int i = 0; i < n_axis; i++) {
        display.data->setAxis(i, axes[i]);
    }
    for (int i = 0; i < n_axis; i++) {
        display.data->setLimit(i, limits[i]);
    }
}

extern "C" void show_limits(bool probe, const bool* limits, size_t n_axis) {
    display.data->setNAxes(n_axis);
    // limits done with DROs
    if (display.data->isProbe() != probe) {
        display.data->setProbe(probe);
    }
}

extern void show_spindle_coolant(int spindle, bool flood, bool mist) {
    display.data->setMist(mist);
    display.data->setFlood(flood);
    display.data->setSpindleNum(spindle);
}

extern "C" void show_file(const char* filename, file_percent_t percent) {
    display.data->setFname(filename);
    display.data->setPercent(percent);
 
}

extern "C" void begin_status_report() {
        last_report = millis();
        display.data->clearData();
}

extern "C" void end_status_report() {
        nonrepeating_screen_delay(DROScreen, true);
}

extern "C" void show_versions(const char* grbl_version, const char *fluidnc_version) {
    last_report = millis();
    display.data->setGrblVersion(grbl_version);
    display.data->setVersion(fluidnc_version);
    //set_display_delay(radio_delay);
    nonrepeating_screen_delay(SplashScreen, false);
}

extern "C" void handle_msg(char* command, char* arguments) {
   //if (handle_expander_msg(command, arguments)) return;
   if (strlen(command) == 0 || strlen(arguments) == 0) {
        return;
    }
    if (!strcmp(command, "INFO") ) {
        std::string args = arguments;
        std::string subargument;

        if (!strcmp(arguments, " Restarting")) {
            last_report = millis();
            do_Restart();
        }
        size_t pos = 0;
        std::string msg = " FluidNC v";
        if (pos = args.rfind(msg) == 0) {
            last_report = millis();
            subargument = args.substr(msg.length());
            parse_VER(subargument);
            return;
        }
        msg = " Connecting to STA SSID:";
        if (pos = args.rfind(msg) == 0) {
            last_report = millis();
            subargument = args.substr(msg.length());
            DebugSerial.println(subargument.c_str());
            parse_STA(subargument);
            return;
        }
        msg = " Connected - IP is ";
        if (pos = args.rfind(msg) == 0) {
            last_report = millis();
            subargument = args.substr(msg.length());
            parse_IP(subargument);
            return;
        }
        msg = " AP SSID ";
        if (pos = args.rfind(msg) == 0) {
            last_report = millis();
            subargument = args.substr(msg.length());
            parse_AP(subargument);
            return;
        }
        msg = " BT Started with ";
        if (pos = args.rfind(msg) == 0) {
            last_report = millis();
            subargument = args.substr(msg.length());
            parse_BT(subargument);
            return;
        }
        msg = " WebUI: Request from ";
        if (pos = args.rfind(msg) == 0) {
            last_report = millis();
            subargument = args.substr(msg.length());
            parse_WebUI(subargument);
            return;
        }
    } else if (!strncmp(command, "Mode=", strlen("Mode="))) {
        std::string cmd = command;
        std::string args = arguments;
        std::string subargument = cmd.substr(5) + ":" + arguments;
        parse_Mode(subargument);
    }
}

// [MSG:INFO: Connecting to STA:SSID foo]
void parse_STA(std::string &_report) {
    display.data->setRadioType("STA");
    display.data->setRadioInfo(_report);
    //display.data->setRadioAddr("");
    nonrepeating_screen_delay(RadioScreen, false);
}

// [MSG:INFO: Connected - IP is 192.168.68.134]
void parse_IP(std::string &_report) {
    //size_t start = _report.rfind(" ") + 1;
    //if (start >= 1) {
    //display.data->setRadioAddr(_report.substr(start, _report.size() - start - 1));
    display.data->setRadioAddr(_report);
    nonrepeating_screen_delay(RadioScreen, false);
}

// [MSG:INFO: AP SSID foo IP 192.168.68.134 mask foo channel foo]
void parse_AP(std::string &_report) {
    size_t ssid_end = _report.rfind(" IP ");
    size_t ip_end   = _report.rfind(" mask ");
    size_t ip_start = ssid_end + strlen(" IP ");

    display.data->setRadioType("AP");
    if (ssid_end > 0) {
        display.data->setRadioInfo(_report.substr(0, ssid_end));
        if (ip_end > 0) {
            display.data->setRadioAddr(_report.substr(ip_start, ip_end - ip_start));
            nonrepeating_screen_delay(RadioScreen, false);
            return;
        }
    }
}
//[MSG: Mode=STA:SSID=TP-LINK_719B:Status=Connected:IP=192.168.29.103:MAC=D8-BC-38-57-1E-B8]
void parse_Mode(std::string &_report) {
    size_t mode_end = _report.rfind(":SSID=");
    size_t ssid_end = _report.rfind(":Status=");
    size_t ip_end   = _report.rfind(":MAC=");
    size_t ip_start = _report.rfind(":IP=");
    //debug_println("mode");
    if(mode_end > 0) {
        display.data->setRadioType(_report.substr(0, mode_end));
        if (ssid_end > 0) {
            display.data->setRadioInfo(_report.substr(mode_end + 6, ssid_end -(mode_end+ 6)));
            if (ip_end >0  && ip_start > 0) {
                display.data->setRadioAddr(_report.substr(ip_start +4, ip_end - (ip_start +4)));
                nonrepeating_screen_delay(RadioScreen, false);
                return;
            }
        } 
    }
}  
void parse_BT(std::string &_report) {
   std::string btname = _report;
   display.data->setRadioType("BT");
   display.data->setRadioInfo (btname);
   nonrepeating_screen_delay(RadioScreen, false);
}

void parse_WebUI(std::string &_report) {
    display.data->setWebUIAddr(_report);
    nonrepeating_screen_delay(WebUIScreen, true);
}

// [MSG:INFO: Connecting to STA:SSID foo]
void parse_VER(std::string &_report) {
    display.data->setVersion("v"+ _report.substr(0, _report.find(" ") ));
    //set_display_delay(radio_delay);
    nonrepeating_screen_delay(SplashScreen, false);

}

// [MSG: Restarting]
void do_Restart() {
    display.data->clearAllData();
    display.clearSatisfaction();
    initial_msg_loop = true;
    nonrepeating_screen_delay(SplashScreen, true);
    //set_display_delay(radio_delay);
    fnc_wait_ready();  // Synchronize with FluidNC
    fnc_putchar('?');  // Initial status report
}


extern "C" int fnc_getchar() {
    if (FNCSerial.available()) {
        return FNCSerial.read();
    }
    return -1;
}
extern "C" void fnc_putchar(uint8_t c) {
    FNCSerial.write(c);
}
extern "C" void debug_putchar(char c) {
    DebugSerial.write(c);
}
extern "C" void debug_println(const char * c) {
    DebugSerial.println(c);
}
extern "C" int milliseconds() {
    return millis();
}


void setup() {
    //pinMode(PIN_POWER_ON, OUTPUT);
    //digitalWrite(PIN_POWER_ON, HIGH);
    //pinMode(PIN_BUTTON_2, INPUT);  // active low has physical pullup and RC

#ifdef DEBUG_USB
    DebugSerial.begin(115200);  // used for debugging
    delay(2000);                // delay to allow USB connection of PC to connect
    DebugSerial.println("Begin T-Display-S3");
#endif

    FNCSerial.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);  // connected to FluidNC

    display.begin();
    //display.getScreen(SplashScreen)->show();
    fnc_send_line("$Build/Info", 3000);
    nonrepeating_screen_delay(SplashScreen, true);
    //set_display_delay(radio_delay);
 }

void doNextScreen() {
    delaying = false;
    initial_msg_loop = false;
    satisfaction_delaying = false;
    display_delay = 0;
    if (!toBeDisplayed.empty()) {
        auto last = toBeDisplayed.front();
        auto lastp = last.lock();
        toBeDisplayed.pop_front();
        bool satisfied = lastp->show();
        //currently_displaying = last;
        display.setCurrentScreen(last);
        if (lastp->needsWait()) (radio_delay, satisfied);
    }
}

void loop() {
    uint32_t last_request = 0;
    uint32_t last_radio = 0;
    uint32_t retries = 0;
    fnc_wait_ready();  // Synchronize with FluidNC
    fnc_putchar('?');  // Initial status report
    while (1) {
        fnc_poll();
        if (delaying) {
            if (millis() -last_delay_start > display_delay) {
                if (satisfaction_delaying)  {
                        uint32_t delayms = initial_msg_loop ? initial_satisfaction_delay : satisfaction_delay;
                        if (millis() - satisfaction_start > delayms) {
                            //give up
                            debug_println("Satisfaction timeout");
                            doNextScreen();
                        }
                } else {
                    doNextScreen();
                }
            }
        } else if (satisfaction_delaying) {
                debug_println("This should never happen");
        }
        if (millis() - last_report  > max_report_interval) {
            if (millis() - last_request > request_retry_interval) {
                if (retries > 1 && !initial_msg_loop) {
                    display.data->clearAllData();
                    display.clearSatisfaction();
                    nonrepeating_screen_delay(DROScreen, false);
                }
                fnc_putchar('?');  // Initial status report
                last_request = millis();
                retries++;
            }          
        } else {
            retries = 0;
        }
        if (millis() - last_radio > 5000) {
            auto rs = display.getScreen(RadioScreen);
            auto cd = display.getCurrentScreen();
            auto rsp = rs.lock();
            auto cdp = cd.lock();
            if (cdp && rsp  && cdp != rsp && !rsp->isSatisfied()) {
                fnc_send_line("$Build/Info", 3000);
            }
            last_radio = millis();
        }
    }
}

void readButtons() {
    int bPressed = display.getPressedButton();
    if (bPressed == -1) return;
    if ((display.getNumButtons() ==1 && bPressed == 0) ||
        (display.getNumButtons() ==3 && bPressed == 2)) {
        if (display.data->isRunState()) {
            debug_putchar('!');
            fnc_putchar('!');
        } else if (display.data->isHoldState()) {
            debug_putchar('~');
            fnc_putchar('~');
        }
    } else if (display.getNumButtons() ==2 && bPressed == 0) {
        if (display.data->isRunState()) {
            debug_putchar('!');
            fnc_putchar('!');
        } 
    } else if (display.getNumButtons() ==2 && bPressed == 1) {
        if (display.data->isHoldState()) {
            debug_putchar('~');
            fnc_putchar('~');
        }
    }
}

void readEncoder() {
    if (int encval = display.getEncoder() != -1) {
        debug_println(std::to_string(encval).c_str());
    } 
}

extern "C" void poll_extra() {
#ifdef DEBUG_USB
    while (DebugSerial.available()) {
        char c = DebugSerial.read();
        if (c != '\r') {
            debug_putchar(c);
            putchar(c);
        }
    }
#endif
    readButtons();
    readEncoder();
}
