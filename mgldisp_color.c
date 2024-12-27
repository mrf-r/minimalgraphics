#include "mgl.h"

// template example of how to use this library with RGB565 display
//

__attribute__((weak)) void mglDispSetZone(const uint16_t wax, const uint16_t way, const uint16_t wax_size, const uint16_t way_size)
{
    uint16_t waxe = wax + wax_size;
    uint16_t waye = way + way_size;
    MGL_ASSERT(wax < DISPLAY_SIZE_X);
    MGL_ASSERT(waxe <= DISPLAY_SIZE_X);
    MGL_ASSERT(way < DISPLAY_SIZE_Y);
    MGL_ASSERT(waye <= DISPLAY_SIZE_Y);
    // send SPI command to fill the zone left->right top->bottom
    displaySetZone(wax, waxe, way, waye);
}

typedef union {
    uint16_t word;
    struct {
        uint16_t red : 5;
        uint16_t green : 6;
        uint16_t blue : 5;
    };
} DisplayActualColor;

__attribute__((weak)) void mglDispPixelOut(MglColor c)
{
    DisplayActualColor dc;
#if DITHERING_DISABLED
    dc.red = c.red >> 3;
    dc.green = c.green >> 2;
    dc.blue = c.blue >> 3;
#else // DITHERING_ENABLED
    static uint32_t lcg = 0;
    lcg = lcg * 1103515245 + 12345;
    int32_t red = c.red + ((lcg >> 20) & 0x7);
    dc.red = (red < 255 ? red : 255) >> 3;
    int32_t green = c.green + ((lcg >> 23) & 0x7);
    dc.green = (green < 255 ? green : 255) >> 2;
    int32_t blue = c.blue + ((lcg >> 17) & 0x7);
    dc.blue = (blue < 255 ? blue : 255) >> 3;
#endif
    // send SPI command to transfer pixel to display RAM
    displaySendPixel(dc.word);
}

__attribute__((weak)) void mglDispUpdate()
{
    // this method is not used for multi-color displays unless you want to implement framebuffer
}

#ifndef MGL_SINGLEDISPLAY
static MglDispContext context;
const MglDisplay mgl_display = {
    .context = &context,
    .size_x = DISPLAY_SIZE_X,
    .size_y = DISPLAY_SIZE_Y,
    .setZone = mglDispSetZone,
    .pixelOut = mglDispPixelOut,
    .update = mglDispUpdate
};
#endif
