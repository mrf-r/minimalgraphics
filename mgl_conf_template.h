#ifndef __MGL_CONF_H
#define __MGL_CONF_H

#ifdef DEBUG
#include <assert.h>
#define MGL_ASSERT assert
#else
#define MGL_ASSERT(...)
#endif

#define MGL_SINGLEDISPLAY
// #define MGL_BITMAPMIRRORED // deprecated, will be deleted

#define DISPLAY_SIZE_X 128
#define DISPLAY_SIZE_Y 64

#endif // __MGL_CONF_H