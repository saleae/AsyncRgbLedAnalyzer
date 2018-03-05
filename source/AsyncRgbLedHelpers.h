#ifndef ASYNCRGBLED_ANALYZER_HELPERS
#define ASYNCRGBLED_ANALYZER_HELPERS

#include <AnalyzerTypes.h>

enum ColorLayout {
    LAYOUT_RGB = 0,
    LAYOUT_GRB
};

struct RGBValue
{
    RGBValue(U16 r, U16 g, U16 b) :
        red(r), green(g), blue(b) {;}

    U16 red = 0;
    U16 green = 0;
    U16 blue = 0;
    U16 padding = 0; // no alpha in LED colors

    void ConvertToControllerOrder(ColorLayout layout, U16* values) const;

    static RGBValue CreateFromControllerOrder(ColorLayout layout, U16* values);
};


#endif // of #define ASYNCRGBLED_ANALYZER_SETTINGS

