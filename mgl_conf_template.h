#ifndef __MGL_CONF_H
#define __MGL_CONF_H

#ifdef DEBUG
#include <assert.h>
#define MGL_ASSERT assert
#else
#define MGL_ASSERT(...)
#endif

#define MGL_SINGLEDISPLAY

// colors for monochrome
#define COLOR_OFF ((MglColor){.wrd=0x0})
#define COLOR_ON ((MglColor){.wrd=0x1})
#define COLOR_INVERT ((MglColor){.wrd=0x1000})

#define DISPLAY_SIZE_X 128
#define DISPLAY_SIZE_Y 64

#endif // __MGL_CONF_H