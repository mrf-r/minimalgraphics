# minimal graphic library

- For color, grayscale or monochrome displays
- Supports multiple displays (but also contains simple singledisplay mode)
- Designed for low throughput bus with framebuffer on the panel side (write-only)
- proportional fonts (please `do not` use fnt_x.h fonts in your projects)
- overwrites text, can also overwtite entire line within the working area (canvas). no need to clean.
- default colors are ARGB 8888. You can implement static dither for 565 panels inside `pixelOut` (i'll try to add example later)
- contains HSV-RGB converter

## usage

Please, provide in `mgl_conf.h`:

- `#define MGL_ASSERT()`
- `#define MGL_SINGLEDISPLAY` if you don't need multiple
- `DISPLAY_SIZE_X` `DISPLAY_SIZE_Y` for default single display implementation
- your own fonts (TBD)
- for multimple displays you need to create `MglDisplay` struct for each
    - `setZone` is the command for limiting the canvas. pixels expected sequence is left->right, top->bottom. This command has hardware analog on most tft controllers, so you need to translate it to one SPI/IIC request. In case of monocrome lcd, same behavior is emulates for framebuffer.
    - `pixelOut` should write one pixel - one SPI/IIC transaction for most color screens
    - `update` is just a wrapper, it may be used for triggering local framebuffer to panel transfer in case this process is application-driven
- you can use https://www.pentacom.jp/pentacom/bitfontmaker2 for font editing. use `python3 convert.py fontname.json` for export to `fontname.c` file.
- use `rearrange.py` to convert existing fonts to bitfontmaker2 format (manual tuning needed, paste array to file)

## TODO

- FONTS!!! add more and probably adopt this lib to common free fonts
    - convert.py save font image
    - clean - check if all of them are ok and delete if not, get others from hdd ?
- lines and circles ??
- add color dithered examples
