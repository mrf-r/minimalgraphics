#ifndef _MGL_H
#define _MGL_H

// MINIMAL GRAPHIC LIBRARY

#include "mgl_conf.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wpedantic"
typedef union {
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    };
    uint32_t wrd;
} MglColor;
// #pragma GCC diagnostic pop

static const MglColor COLOR_OFF = {.wrd = 0};
static const MglColor COLOR_ON = {.wrd = 1};
static const MglColor COLOR_INVERT = {.wrd = 1};

typedef enum {
    MGL_ALIGN_LEFT = 0,
    MGL_ALIGN_CENTER,
    MGL_ALIGN_RIGHT
} MglAlignEn;

typedef const struct
{
    const void* bitmap_data_horiz; // horizontal fill one pixel
    // void* bitmap_data_vert; // horizontal fill vertical line of 8 pixels
    const uint8_t* symbol_width; // char width array (if 0 then monospaced)
    uint8_t bmp_width; // number of vertical bytes/words / horizontal type (byte/halfword/word)
    uint8_t bmp_height; // number of horizontal bytes/words / vertical type
    uint8_t startchar; // start char (0x0 or 0x20)
} MglFont;

#ifndef MGL_SINGLEDISPLAY
typedef struct
{
    int16_t pos_x;
    int16_t pos_y;
    uint16_t waxs;
    uint16_t waxe;
    uint16_t ways;
    uint16_t waye;
    const MglFont* font;
    MglAlignEn walign;
    MglColor color_back;
} MglDispContext;

typedef const struct
{
    MglDispContext* context;
    uint16_t size_x;
    uint16_t size_y;
    void (*setZone)(const uint16_t wax, const uint16_t way, const uint16_t wax_size, const uint16_t way_size);
    void (*pixelOut)(MglColor c);
    void (*update)(void);
} MglDisplay;
#endif // MGL_SINGLEDISPLAY

void mgsWorkingArea(const uint16_t x, const uint16_t y, const uint16_t xsize, const uint16_t ysize);
void mgsCursorAbs(const int16_t x, const int16_t y);
void mgsCursorRel(const int16_t x, const int16_t y);
void mgsFont(const MglFont* f);
void mgsAlign(MglAlignEn align);
void mgsDisplayUpdate(void);
#ifndef MGL_SINGLEDISPLAY
void mgsDisplay(const MglDisplay* d);
#endif // MGL_SINGLEDISPLAY
void mgsBackColor(MglColor color);
///////////////////////////////////////////////////////////////////////////////////////////
void mgdFill(MglColor color);
void mgdFillPattern(const uint8_t* bitmap, const uint8_t pixmod, MglColor color);
void mgdBitmap(const void* bitmap, const uint8_t bmpsize, const uint8_t width, const uint8_t height, MglColor color);
void mgdChar(const char c, MglColor color);
void mgdString(const char* str, MglColor color);
uint16_t mgStringLengthGet(const char* str);
void mgdStringLine(const char* str, MglColor color);
void mgdHex32(uint32_t v, MglColor color);
void mgdHex16(uint16_t v, MglColor color);

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
MglColor mgColorRgb(uint8_t red, uint8_t green, uint8_t blue);
MglColor mgColorHsv(uint8_t hue, uint8_t saturation, uint8_t value);
MglColor mgAlphablend(uint16_t intensity, MglColor lowest, MglColor highest);
MglColor mgShine(uint16_t intensity, MglColor highest, MglColor lowest);
void mgdHsvTestFill();
void mgdHsvTestFill2();

#ifdef __cplusplus
}
#endif

#endif /* _MGL_H */
