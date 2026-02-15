# minimal graphic library

- For color, grayscale or monochrome displays
- Designed for low throughput bus with framebuffer on the panel side (write-only). But not limited to
- Supports multiple displays (also contains simple singledisplay mode)
- proportional fonts
- overwrites text, can also overwtite entire line within the working area (canvas). no need to clean.
- default colors are ARGB 8888. You can implement any color conversion for any type of display as well as static dither for 565 panels inside `pixelOut` method
- contains HSV-RGB converter

## usage

Please, provide:

- `mgl_conf.h` file
    - `#define MGL_ASSERT()`
    - `#define MGL_SINGLEDISPLAY` if you don't need multiple
    - `DISPLAY_SIZE_X` `DISPLAY_SIZE_Y` for default single display implementation
- display low level driver file (like `mgldisp_color.c`)
    - `setZone` is the command for limiting the canvas. pixels expected sequence is left->right, top->bottom. This command has hardware analog on most tft controllers, so you need to translate it to one SPI/IIC request. In case of monocrome lcd, same behavior should be emulates for framebuffer.
    - `pixelOut` should write one pixel - one SPI/IIC transaction for most color screens, or single bit for monochrome framebuffer. Dithering may be used for 565 or other low-depth panels.
    - `update` is just a wrapper, it may be used for triggering local framebuffer to panel transfer in case this process is application-driven
    - for multimple displays you need to create `MglDisplay` struct for each
- your own fonts files
    - `fonts/conv.py` can be used in interactive mode to convert fonts from raw arrays or json, image preview, and export .c files
    - https://www.pentacom.jp/pentacom/bitfontmaker2 can be used for json editing

## TODO

- do we really need lines and circles?
