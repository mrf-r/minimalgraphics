#include "mgl.h"

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

#ifdef MGL_SINGLEDISPLAY
static const MglDisplay* const disp = MGL_SINGLEDISPLAY;
#else
static const MglDisplay* disp = 0;
#endif
static MglColor color_back;

void mgsWorkingArea(const uint16_t x, const uint16_t y, const uint16_t xsize, const uint16_t ysize)
{
    ASSERT(disp);
    ASSERT((x >= 0) && (y >= 0) && (xsize > 0) && (ysize > 0));
    if ((x < disp->size_x) && (y < disp->size_y)) {
        waxs = x;
        ways = y;
        waxe = x + xsize;
        waye = y + ysize;
        if (waxe > disp->size_x) {
            ASSERT(0);
            waxe = disp->size_x;
        }
        if (waye > disp->size_y) {
            ASSERT(0);
            waye = disp->size_y;
        }
        pos_x = x;
        pos_y = y;
    } else {
        ASSERT(0);
        waxs = 0;
        waxe = 0;
        ways = 0;
        waye = 0;
    }
}

void mgsCursorAbs(const int16_t x, const int16_t y)
{
    pos_x = x;
    pos_y = y;
}

void mgsCursorRel(const int16_t x, const int16_t y)
{
    pos_x += x;
    pos_y += y;
}

void mgsFont(const MglFont* f)
{
    font = f;
}

void mgsAlign(MglAlignEn align)
{
    walign = align;
}

void mgsDisplayUpdate(const MglDisplay* d)
{
    d->update();
}

void mgsDisplay(const MglDisplay* d)
{
#ifdef MGL_SINGLEDISPLAY
    ASSERT(0);
#else
    disp = d;
    mgsWorkingArea(0, 0, disp->size_x, disp->size_y);
#endif
}

void mgsBackColor(MglColor color)
{
    color_back = color;
}

///////////////////////////////////////////////////////////////////////////////////////////

void mgdFill(MglColor color)
{
    ASSERT(disp);
    uint16_t xsize = waxe - waxs;
    uint16_t ysize = waye - ways;
    disp->setZone(waxs, ways, xsize, ysize);
    uint32_t pixels = xsize * ysize;
    for (uint32_t i = 0; i < pixels; i++)
        disp->pixelOut(color);
}

void mgdFillPattern(const uint8_t* bitmap, const uint8_t pixmod, MglColor color)
{
    ASSERT(disp);
    uint8_t bmppos = 0;
    uint16_t xsize = waxe - waxs;
    uint16_t ysize = waye - ways;
    disp->setZone(waxs, ways, xsize, ysize);
    uint32_t pixels = xsize * ysize;
    for (uint32_t i = 0; i < pixels; i++) {
        if (bitmap[bmppos >> 3] & (1 << (bmppos & 7)))
            disp->pixelOut(color);
        else
            disp->pixelOut(color_back);
        bmppos++;
        if (bmppos == pixmod)
            bmppos = 0;
    }
}

void mgdBitmap(const void* bitmap, const uint8_t bmpsize, const uint8_t width, const uint8_t height, MglColor color)
{
    ASSERT(disp);
    uint16_t xstart = pos_x;
    uint16_t xend = pos_x + width;
    uint16_t ystart = pos_y;
    uint16_t yend = pos_y + height;
    if (xstart < waxs)
        xstart = waxs;
    if (xend > waxe)
        xend = waxe;
    if (ystart < ways)
        ystart = ways;
    if (yend > waye)
        yend = waye;
    if ((xstart < xend) && (ystart < yend)) {
        disp->setZone(xstart, ystart, xend - xstart, yend - ystart);
        for (uint16_t iy = ystart; iy < yend; iy++) {
            uint32_t bmpline; // left aligned line
            uint16_t bmppos = iy - pos_y;
            if (bmpsize > 8) {
                if (bmpsize > 16)
                    bmpline = ((uint32_t*)bitmap)[bmppos];
                else
                    bmpline = ((uint16_t*)bitmap)[bmppos] << 16;
            } else
                bmpline = ((uint8_t*)bitmap)[bmppos] << 24;
            for (uint16_t ix = xstart; ix < xend; ix++) {
                uint16_t pixoff = ix - pos_x;
                if (pixoff < bmpsize)
                    disp->pixelOut((bmpline >> (31 - pixoff)) & 0x1 ? color : color_back);
                // disp->pixelOut((bmpline >> pixoff) & 0x1 ? color : color_back);
                else
                    disp->pixelOut(color_back);
            }
        }
    }
    pos_x += width;
}

void mgdChar(const char c, MglColor color)
{
    ASSERT(disp);
    ASSERT(font);
    uint8_t charpos = c - font->startchar;
    uint8_t width = font->symbol_width[charpos];
    uint8_t height = font->bmp_height;
    uint8_t bmpmul;
    if (font->bmp_width > 8) {
        if (font->bmp_width > 16)
            bmpmul = 4;
        else
            bmpmul = 2;
    } else {
        bmpmul = 1;
    }
    const void* bitmap = font->bitmap_data_horiz + charpos * font->bmp_height * bmpmul;
    mgdBitmap(bitmap, font->bmp_width, width, height, color);
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
    ASSERT(font);
    uint16_t strlength = 0;
    while (*str) {
        uint8_t c = *str - font->startchar;
        strlength += font->symbol_width[c];
        str++;
    }
    return strlength;
}

void mgdStringLine(const char* str, MglColor color)
{
    ASSERT(disp);
    ASSERT(font);
    int16_t text_start;
    int16_t pos_x_buf = pos_x;
    uint8_t height = font->bmp_height;
    switch (walign) {
    case MGL_ALIGN_LEFT:
        text_start = pos_x;
        break;
    case MGL_ALIGN_CENTER:
        text_start = pos_x - mgStringLengthGet(str) / 2;
        break;
    case MGL_ALIGN_RIGHT:
        text_start = pos_x - mgStringLengthGet(str);
        break;
    default:
        ASSERT(0);
        break;
    }
    // start string
    if (MGL_ALIGN_LEFT != walign) {
        int xstart = waxs;
        int xend = text_start;
        int ystart = pos_y;
        int yend = pos_y + height;
        if (xend > waxe)
            xend = waxe;
        if (ystart < ways)
            ystart = ways;
        if (yend > waye)
            yend = waye;
        if ((xstart < xend) && (ystart < yend)) {
            uint16_t xsize = xend - xstart;
            uint16_t ysize = yend - ystart;
            uint32_t pixels = xsize * ysize;
            disp->setZone(xstart, ystart, xsize, ysize);
            for (uint32_t i = 0; i < pixels; i++)
                disp->pixelOut(color_back);
        }
    }
    // print string
    pos_x = text_start;
    mgdString(str, color);
    // finalize string
    if (MGL_ALIGN_RIGHT != walign) {
        int xstart = pos_x;
        int xend = waxe;
        int ystart = pos_y;
        int yend = pos_y + height;
        if (xstart < waxs)
            xstart = waxs;
        if (ystart < ways)
            ystart = ways;
        if (yend > waye)
            yend = waye;
        if ((xstart < xend) && (ystart < yend)) {
            uint16_t xsize = xend - xstart;
            uint16_t ysize = yend - ystart;
            uint32_t pixels = xsize * ysize;
            disp->setZone(xstart, ystart, xsize, ysize);
            for (uint32_t i = 0; i < pixels; i++)
                disp->pixelOut(color_back);
        }
    }
    // shift cursor pos
    pos_x = pos_x_buf;
    pos_y += height;
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
    ASSERT(disp);
    uint16_t x, y;
    // first
    disp->setZone(0, 0, disp->size_x, disp->size_y);
    for (y = 0; y < disp->size_y; y++)
        for (x = 0; x < disp->size_x; x++)
            disp->pixelOut(mgColorHsv(x * 256 / disp->size_x, y * 256 / disp->size_y, 128));
}

void mgdHsvTestFill2()
{
    ASSERT(disp);
    uint16_t x, y;
    // second
    disp->setZone(0, 0, disp->size_x, disp->size_y);
    for (y = 0; y < disp->size_y; y++)
        for (x = 0; x < disp->size_x; x++)
            disp->pixelOut(mgColorHsv(x * 256 / disp->size_x, 255, y * 128 / disp->size_y + 128));
}
