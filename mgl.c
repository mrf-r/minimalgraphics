#include "mgl.h"

#ifdef MGL_SINGLEDISPLAY
#define ASSERTDISP()

static int16_t pos_x;
static int16_t pos_y;
// working area X, Y - start, end
// end points to the next pixel, that is not affected
// ex. if we want to edit 4 pixels, then waxs = 10, waxe = 14 (not 13)
static uint16_t waxs;
static uint16_t waxe;
static uint16_t ways;
static uint16_t waye;
static const MglFont* font;
static MglAlignEn walign;
static MglColor color_back;
void mglDispSetZone(const uint16_t wax, const uint16_t way, const uint16_t wax_size, const uint16_t way_size);
void mglDispPixelOut(MglColor c);
void mglDispUpdate(void);
#define POS_X pos_x
#define POS_Y pos_y
#define WAXS waxs
#define WAXE waxe
#define WAYS ways
#define WAYE waye
#define FONTP font
#define ALIGN walign
#define COLORBACK color_back
#define SIZE_X (DISPLAY_SIZE_X)
#define SIZE_Y (DISPLAY_SIZE_Y)
#define SETZONE(x, y, xs, ys) mglDispSetZone(x, y, xs, ys)
#define PIXELOUT(c) mglDispPixelOut(c)
#define UPDATE(c) mglDispUpdate(c)

#else // MGL_SINGLEDISPLAY
static const MglDisplay* disp = 0;
#define ASSERTDISP() MGL_ASSERT(disp)

#define POS_X (disp->context->pos_x)
#define POS_Y (disp->context->pos_y)
#define WAXS (disp->context->waxs)
#define WAXE (disp->context->waxe)
#define WAYS (disp->context->ways)
#define WAYE (disp->context->waye)
#define FONTP (disp->context->font)
#define ALIGN (disp->context->walign)
#define COLORBACK (disp->context->color_back)
#define SIZE_X (disp->size_x)
#define SIZE_Y (disp->size_y)
#define SETZONE(x, y, xs, ys) disp->setZone(x, y, xs, ys)
#define PIXELOUT(c) disp->pixelOut(c)
#define UPDATE() disp->update()

#endif // MGL_SINGLEDISPLAY

void mgsWorkingArea(const uint16_t x, const uint16_t y, const uint16_t xsize, const uint16_t ysize)
{
    ASSERTDISP();
    MGL_ASSERT((x >= 0) && (y >= 0) && (xsize > 0) && (ysize > 0));
    if ((x < SIZE_X) && (y < SIZE_Y)) {
        WAXS = x;
        WAYS = y;
        WAXE = x + xsize;
        WAYE = y + ysize;
        if (WAXE > SIZE_X) {
            MGL_ASSERT(0);
            WAXE = SIZE_X;
        }
        if (WAYE > SIZE_Y) {
            MGL_ASSERT(0);
            WAYE = SIZE_Y;
        }
        POS_X = x;
        POS_Y = y;
    } else {
        MGL_ASSERT(0);
        WAXS = 0;
        WAXE = 0;
        WAYS = 0;
        WAYE = 0;
    }
}

void mgsCursorAbs(const int16_t x, const int16_t y)
{
    ASSERTDISP();
    POS_X = x;
    POS_Y = y;
}

void mgsCursorRel(const int16_t x, const int16_t y)
{
    ASSERTDISP();
    POS_X += x;
    POS_Y += y;
}

void mgsFont(const MglFont* f)
{
    ASSERTDISP();
    FONTP = f;
}

void mgsAlign(MglAlignEn align)
{
    ASSERTDISP();
    ALIGN = align;
}

void mgsDisplayUpdate()
{
    ASSERTDISP();
    UPDATE();
}

#ifndef MGL_SINGLEDISPLAY
void mgsDisplay(const MglDisplay* d)
{
    disp = d;
    ASSERT(d->context);
    ASSERT(d->size_x);
    ASSERT(d->size_y);
    ASSERT(d->setZone);
    ASSERT(d->pixelOut);
    mgsWorkingArea(0, 0, SIZE_X, SIZE_Y);
}
#endif // MGL_SINGLEDISPLAY

void mgsBackColor(MglColor color)
{
    ASSERTDISP();
    COLORBACK = color;
}

///////////////////////////////////////////////////////////////////////////////////////////

void mgdFill(MglColor color)
{
    ASSERTDISP();
    uint16_t xsize = WAXE - WAXS;
    uint16_t ysize = WAYE - WAYS;
    SETZONE(WAXS, WAYS, xsize, ysize);
    uint32_t pixels = xsize * ysize;
    for (uint32_t i = 0; i < pixels; i++)
        PIXELOUT(color);
}

void mgdFillPattern(const uint8_t* bitmap, const uint8_t pixmod, MglColor color)
{
    ASSERTDISP();
    uint8_t bmppos = 0;
    uint16_t xsize = WAXE - WAXS;
    uint16_t ysize = WAYE - WAYS;
    SETZONE(WAXS, WAYS, xsize, ysize);
    uint32_t pixels = xsize * ysize;
    for (uint32_t i = 0; i < pixels; i++) {
        if (bitmap[bmppos >> 3] & (1 << (bmppos & 7)))
            PIXELOUT(color);
        else
            PIXELOUT(COLORBACK);
        bmppos++;
        if (bmppos == pixmod)
            bmppos = 0;
    }
}

void mgdBitmap(const void* bitmap, const uint8_t bmpsize, const uint8_t width, const uint8_t height, MglColor color)
{
    ASSERTDISP();
    uint16_t xstart = POS_X;
    uint16_t xend = POS_X + width;
    uint16_t ystart = POS_Y;
    uint16_t yend = POS_Y + height;
    if (xstart < WAXS)
        xstart = WAXS;
    if (xend > WAXE)
        xend = WAXE;
    if (ystart < WAYS)
        ystart = WAYS;
    if (yend > WAYE)
        yend = WAYE;
    if ((xstart < xend) && (ystart < yend)) {
        SETZONE(xstart, ystart, xend - xstart, yend - ystart);
        for (uint16_t iy = ystart; iy < yend; iy++) {
            uint32_t bmpline; // left aligned line
            uint16_t bmppos = iy - POS_Y;
            if (bmpsize > 8) {
                if (bmpsize > 16)
                    bmpline = ((uint32_t*)bitmap)[bmppos];
                else
                    bmpline = ((uint16_t*)bitmap)[bmppos] << 16;
            } else
                bmpline = ((uint8_t*)bitmap)[bmppos] << 24;
            for (uint16_t ix = xstart; ix < xend; ix++) {
                uint16_t pixoff = ix - POS_X;
                if (pixoff < bmpsize)
                    PIXELOUT((bmpline >> (31 - pixoff)) & 0x1 ? color : COLORBACK);
                // PIXELOUT((bmpline >> pixoff) & 0x1 ? color : COLORBACK);
                else
                    PIXELOUT(COLORBACK);
            }
        }
    }
    POS_X += width;
}

void mgdChar(const char c, MglColor color)
{
    ASSERTDISP();
    MGL_ASSERT(FONTP);
    uint8_t charpos = c - FONTP->startchar;
    uint8_t width = FONTP->symbol_width ? FONTP->symbol_width[charpos] : FONTP->bmp_width;
    uint8_t height = FONTP->bmp_height;
    uint8_t bmpmul;
    if (FONTP->bmp_width > 8) {
        if (FONTP->bmp_width > 16)
            bmpmul = 4;
        else
            bmpmul = 2;
    } else {
        bmpmul = 1;
    }
    const void* bitmap = (uint8_t*)FONTP->bitmap_data_horiz + charpos * FONTP->bmp_height * bmpmul;
    mgdBitmap(bitmap, FONTP->bmp_width, width, height, color);
}

void mgdString(const char* str, MglColor color)
{
    const char* ch = str;
    while (*ch) {
        mgdChar(*ch, color);
        ch++;
    }
}

uint16_t mgStringLengthGet(const char* str)
{
    ASSERTDISP();
    MGL_ASSERT(FONTP);
    uint16_t strlength = 0;
    while (*str) {
        uint8_t c = *str - FONTP->startchar;
        strlength += FONTP->symbol_width ? FONTP->symbol_width[c] : FONTP->bmp_width;
        str++;
    }
    return strlength;
}

void mgdStringLine(const char* str, MglColor color)
{
    ASSERTDISP();
    MGL_ASSERT(FONTP);
    int16_t text_start;
    int16_t pos_x_buf = POS_X;
    uint8_t height = FONTP->bmp_height;
    switch (ALIGN) {
    case MGL_ALIGN_LEFT:
        text_start = POS_X;
        break;
    case MGL_ALIGN_CENTER:
        text_start = POS_X - mgStringLengthGet(str) / 2;
        break;
    case MGL_ALIGN_RIGHT:
        text_start = POS_X - mgStringLengthGet(str);
        break;
    default:
        MGL_ASSERT(0);
        break;
    }
    // start string
    if (MGL_ALIGN_LEFT != ALIGN) {
        int xstart = WAXS;
        int xend = text_start;
        int ystart = POS_Y;
        int yend = POS_Y + height;
        if (xend > WAXE)
            xend = WAXE;
        if (ystart < WAYS)
            ystart = WAYS;
        if (yend > WAYE)
            yend = WAYE;
        if ((xstart < xend) && (ystart < yend)) {
            uint16_t xsize = xend - xstart;
            uint16_t ysize = yend - ystart;
            uint32_t pixels = xsize * ysize;
            SETZONE(xstart, ystart, xsize, ysize);
            for (uint32_t i = 0; i < pixels; i++)
                PIXELOUT(COLORBACK);
        }
    }
    // print string
    POS_X = text_start;
    mgdString(str, color);
    // finalize string
    if (MGL_ALIGN_RIGHT != ALIGN) {
        int xstart = POS_X;
        int xend = WAXE;
        int ystart = POS_Y;
        int yend = POS_Y + height;
        if (xstart < WAXS)
            xstart = WAXS;
        if (ystart < WAYS)
            ystart = WAYS;
        if (yend > WAYE)
            yend = WAYE;
        if ((xstart < xend) && (ystart < yend)) {
            uint16_t xsize = xend - xstart;
            uint16_t ysize = yend - ystart;
            uint32_t pixels = xsize * ysize;
            SETZONE(xstart, ystart, xsize, ysize);
            for (uint32_t i = 0; i < pixels; i++)
                PIXELOUT(COLORBACK);
        }
    }
    // shift cursor pos
    POS_X = pos_x_buf;
    POS_Y += height;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

const char h2d[17] = "0123456789ABCDEF";

void mgdHex32(uint32_t v, MglColor color)
{
    mgdChar(h2d[(v >> 28) & 0xF], color);
    mgdChar(h2d[(v >> 24) & 0xF], color);
    mgdChar(h2d[(v >> 20) & 0xF], color);
    mgdChar(h2d[(v >> 16) & 0xF], color);
    mgdChar(h2d[(v >> 12) & 0xF], color);
    mgdChar(h2d[(v >> 8) & 0xF], color);
    mgdChar(h2d[(v >> 4) & 0xF], color);
    mgdChar(h2d[v & 0xF], color);
}
void mgdHex16(uint16_t v, MglColor color)
{
    mgdChar(h2d[(v >> 12) & 0xF], color);
    mgdChar(h2d[(v >> 8) & 0xF], color);
    mgdChar(h2d[(v >> 4) & 0xF], color);
    mgdChar(h2d[v & 0xF], color);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

MglColor mgColorRgb(uint8_t red, uint8_t green, uint8_t blue)
{
    MglColor c;
    c.red = red;
    c.green = green;
    c.blue = blue;
    return c;
}

MglColor mgColorHsv(uint8_t hue, uint8_t saturation, uint8_t value)
{
    MglColor c;
    uint32_t red, green, blue;
    if (hue < 86) {
        uint8_t h = hue;
        red = (85 - h) * 771;
        green = h * 771;
        blue = 0;
    } else {
        if (hue < 171) {
            uint8_t h = hue - 85;
            red = 0;
            green = (85 - h) * 771;
            blue = h * 771;
        } else {
            uint8_t h = hue - 170;
            red = h * 771;
            green = 0;
            blue = (85 - h) * 771;
        }
    }
    if (red > 32768)
        red = 32768;
    if (green > 32768)
        green = 32768;
    if (blue > 32768)
        blue = 32768;
    // saturation
    red = red * saturation / 256 + (256 - saturation) * 64;
    green = green * saturation / 256 + (256 - saturation) * 64;
    blue = blue * saturation / 256 + (256 - saturation) * 64;
    // value
    if (value > 128) { // to bright
        value = value - 128;
        red = red * (128 - value) / 128 + value * 256;
        green = green * (128 - value) / 128 + value * 256;
        blue = blue * (128 - value) / 128 + value * 256;
    } else { // to dark
        red = red * value / 128;
        green = green * value / 128;
        blue = blue * value / 128;
    }
    c.red = red >> 7;
    c.green = green >> 7;
    c.blue = blue >> 7;
    return c;
}

MglColor mgAlphablend(uint16_t intensity, MglColor lowest, MglColor highest)
{
    MglColor r;
    int32_t ca = ((highest.red - lowest.red) * intensity) / 65536 + lowest.red;
    r.red = ca;
    ca = ((highest.green - lowest.green) * intensity) / 65536 + lowest.green;
    r.green = ca;
    ca = ((highest.blue - lowest.blue) * intensity) / 65536 + lowest.blue;
    r.blue = ca;
    return r;
}

MglColor mgShine(uint16_t intensity, MglColor highest, MglColor lowest)
{
    MglColor r;
    int32_t ca = highest.red + lowest.red * intensity / 65536;
    r.red = ca > 255 ? 255 : ca;
    ca = highest.green + lowest.green * intensity / 65536;
    r.green = ca > 255 ? 255 : ca;
    ca = highest.blue + lowest.blue * intensity / 65536;
    r.blue = ca > 255 ? 255 : ca;
    return r;
}

void mgdHsvTestFill()
{
    ASSERTDISP();
    uint16_t x, y;
    // first
    SETZONE(0, 0, SIZE_X, SIZE_Y);
    for (y = 0; y < SIZE_Y; y++)
        for (x = 0; x < SIZE_X; x++)
            PIXELOUT(mgColorHsv(x * 256 / SIZE_X, y * 256 / SIZE_Y, 128));
}

void mgdHsvTestFill2()
{
    ASSERTDISP();
    uint16_t x, y;
    // second
    SETZONE(0, 0, SIZE_X, SIZE_Y);
    for (y = 0; y < SIZE_Y; y++)
        for (x = 0; x < SIZE_X; x++)
            PIXELOUT(mgColorHsv(x * 256 / SIZE_X, 255, y * 128 / SIZE_Y + 128));
}
