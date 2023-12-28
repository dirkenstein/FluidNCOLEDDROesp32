#include "displayfuncs.h"
    //DisplayFunctions::DisplayFunctions() : screens {} {data.clearAllData();}

    
    std::string DataBag::DRO_format(int axis, uint32_t val, int decdig) {
        const char * format;
        char   buf[12];
        int    len;
        int    decdiv;
        int    decmul = 1;
        std::string DRO;
        for (int i = 0; i < decdig; i++) decmul *= 10;
        if (isMM()) {
            format = "% 4d.%02d";
            decdiv = 100;
        } else {
            format = "% 3d.%03d";
            decdiv = 10; 
        }

        len = sprintf(buf, format, val/decmul, (val % decmul)/decdiv);

        DRO = buf;
        while (DRO.length() < 9) {
          DRO = " " + DRO;
        }
        DRO = axesNames.substr(axis, /*axis + */ 1) + DRO;
        return DRO;
    }
    
     std::string DataBag::DRO_format(int axis, double val) {
        const char * format;
        char   buf[12];
        int    len;
        int    decdiv;
        std::string DRO;
        std::string axesNames = "XYZABC";

        if (isMM()) {
            format = "% 4.02f";
            decdiv = 100;
        } else {
            format = "% 3.03f";
            decdiv = 10; 
        }

        len = sprintf(buf, format, val);

        DRO = buf;
        while (DRO.length() < 9) {
          DRO = " " + DRO;
        }
        DRO = axesNames.substr(axis, /*axis + */ 1) + DRO;
        return DRO;
    }
