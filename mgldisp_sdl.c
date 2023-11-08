#include "mgl.h"

static uint8_t *framebuffer;
static uint16_t fwaxs;
static uint16_t fways;
static uint16_t fwaxe;
static uint16_t fwaye;
static uint16_t fx;
static uint16_t fy;

static void frmSetZone(const uint16_t wax, const uint16_t way, const uint16_t wax_size, const uint16_t way_size) {
    fwaxs = wax, fways = way, fwaxe = wax + wax_size, fwaye = way + way_size;
    assert(fwaxs < DISPLAY_SIZE_X);
    assert(fwaxe <= DISPLAY_SIZE_X);
    assert(fways < DISPLAY_SIZE_Y);
    assert(fwaye <= DISPLAY_SIZE_Y);
    fx = fwaxs, fy = fways;
}

static void frmOutPixel(MglColor c) {
    framebuffer[fy * DISPLAY_SIZE_X + fx] = c.wrd | 0xFF000000; // no alpha layer
    fx++;
    if (fx >= fwaxe) {
        fx = fwaxs;
        fy++;
        if (fy >= fwaye) {
            fy = fways;
        }
    }
}

const MglDisplay mg_disp_form = {
    .size_x = DISPLAY_SIZE_X,
    .size_y = DISPLAY_SIZE_Y,
    .setZone = frmSetZone,
    .pixelOut = frmOutPixel,
    .update = 0
};
