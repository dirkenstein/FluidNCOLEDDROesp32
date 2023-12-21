#include "displayfuncs.h"
    //DisplayFunctions::DisplayFunctions() : screens {} {data.clearAllData();}

    
    std::string DataBag::DRO_format(int axis, pos_t val) {
        const char * format;
        char   buf[12];
        int    len;
        int    decdiv;
        std::string DRO;
        std::string axesNames = "XYZABC";

        if (isMM()) {
            format = "% 4d.%02d";
            decdiv = 100;
        } else {
            format = "% 3d.%03d";
            decdiv = 10; 
        }

        len = sprintf(buf, format, val/10000, (val % 10000)/decdiv);

        DRO = buf;
        while (DRO.length() < 9) {
          DRO = " " + DRO;
        }
        DRO = axesNames.substr(axis, /*axis + */ 1) + DRO;
        return DRO;
    }
    