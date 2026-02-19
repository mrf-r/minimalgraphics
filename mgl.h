/*
Copyright (C) 2026 Eugene Chernyh (mrf-r)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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

typedef enum {
    MGL_ALIGN_LEFT = 0,
    MGL_ALIGN_CENTER,
    MGL_ALIGN_RIGHT
} MglAlignEn;

typedef const struct
{
    const void* bitmap_data; // horizontal fill one pixel
    const uint8_t* symbol_width; // char width array (if 0 then monospaced)
    uint8_t bmp_width; // also used to determine bitmap_data type
    uint8_t bmp_height;
    uint8_t startchar; // usually 0x20
    uint8_t endchar; // usually 0x80
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

// s - set
void mgsWorkingArea(const uint16_t x, const uint16_t y, const uint16_t xsize, const uint16_t ysize);
void mgsCursorAbs(const int16_t x, const int16_t y);
void mgsCursorRel(const int16_t x, const int16_t y);
void mgsFont(const MglFont* f);
void mgsAlign(MglAlignEn align);
#ifndef MGL_SINGLEDISPLAY
void mgsDisplay(const MglDisplay* d);
#endif // MGL_SINGLEDISPLAY
void mgsBackColor(MglColor color);

// d - draw
void mgdFill(MglColor color);
void mgdFillPattern(const uint8_t* bitmap, const uint8_t pixmod, MglColor color);
void mgdBitmap(const void* bitmap, const uint8_t bmpsize, const uint8_t width, const uint8_t height, MglColor color);
void mgdChar(const char c, MglColor color);
void mgdString(const char* str, MglColor color);
uint16_t mgStringLengthGet(const char* str);
void mgdStringLine(const char* str, MglColor color);
void mgdHex32(uint32_t v, MglColor color);
void mgdHex16(uint16_t v, MglColor color);

// x - extend
void mgxUpdate(void);
void mgxSetZone(uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize);
void mgxPixelOut(MglColor color);

// other stuff
MglColor mgColorRgb(uint8_t red, uint8_t green, uint8_t blue);
MglColor mgColorHsv(uint8_t hue, uint8_t saturation, uint8_t value);
MglColor mgAlphablend(uint8_t intensity, MglColor lowest, MglColor highest);
void mgdHsvTestFill();
void mgdHsvTestFill2();

#ifdef __cplusplus
}
#endif

#endif /* _MGL_H */
