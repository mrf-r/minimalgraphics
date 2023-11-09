# minimal graphic library

- For color, grayscale or monochrome displays
- Supports multiple displays
- Designed for low throughput bus with framebuffer on the panel side (write-only)
- proportional fonts (please `do not` use fnt_x.h fonts in your projects)

## usage

Please, provide:

- `#define ASSERT()`
- `MglColor` typedef
- `DISPLAY_SIZE_X` `DISPLAY_SIZE_Y` for default monochrome implementation
- your own fonts (TBD)
- `#define MGL_SINGLEDISPLAY <your MglDisplay>` in case you are using only one display to save few bytes of ram ;-)

create `MglDisplay` object for your display

- `setZone` is the command for limiting the canvas. pixels expected sequence is left->right, top->bottom. This command has hardware analog on most tft controllers, so you need to translate it to one SPI/IIC request. In case of monocrome lcd, same behavior is emulates for framebuffer.
- `pixelOut` should write one pixel - one SPI/IIC transaction for most color screens
- you are free to encode whatever you want in `MglColor`'s 32 bits (5-6-5, 888, ARGB, 16 levels, 1/0 or some dithering)
- `update` is just a wrapper, it may be used for triggering local framebuffer to panel transfer in case this process is application-driven

## TODO

- cursor/wa context for multiple displays ?
- multiple types of colors ??
- FONTS!!! add more and probably adopt this lib to common free fonts
- lines and circles ??

