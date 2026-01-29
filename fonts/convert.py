# convert font json form https://www.pentacom.jp/pentacom/bitfontmaker2/ to this library
# will not change "true" to "True"

from PIL import Image
import json
import sys
import os

print(os.getcwd())

arg = sys.argv[1]
if arg[:2] == "./" or arg[:2] == ".\\" : arg = arg[2:]
print(f'processing {arg}...')

with open(arg) as jf:
    fontsrc = json.load(jf)
    fontdst = {}
    name = "".join([c for c in arg.split('.')[0] if c.isalpha() or c.isdigit()]) # sorry
    print(f'name: {name}')
    # 0 - determine first and last chars
    firstchar = 255
    lastchar = 0
    for c in fontsrc:
        chnum = int(c)
        if chnum > lastchar: lastchar = chnum
        if chnum < firstchar: firstchar = chnum
    print(f'firstchar: {firstchar}')
    print(f'lastchar: {lastchar}')
    # 1 - normalize charcount to basic ascii
    blank_chars = 0
    for chn in range(firstchar, lastchar):
        if str(chn) in fontsrc:
            # glyph is present, load it and print
            fontdst.update({str(chn): fontsrc[str(chn)]})
        else:
            # no glyph, use default glyph
            fontdst.update({str(chn): [0] * 16})
            print(f'w: char {chr(chn)} is not present in font')
            blank_chars+=1
            pass
    # 2 - normalize the position and find width
    left = 16
    right = 0
    top = 16
    bottom = 0
    charwidth = {}
    for chn in range(firstchar, lastchar):
        glyph = fontdst[str(chn)]
        minpass = False
        mw = 0
        for i, lin in enumerate(glyph):
            if lin != 0:
                # bot
                if i > bottom: bottom = i
                # top
                if not minpass:
                    if i < top: top = i
                    minpass = True
                # left
                ml = f'{lin:016b}'[::-1].index('1')
                if ml < left: left = ml
                # right
                mr = 16 - f'{lin:016b}'.index('1')
                if mr > right: right = mr
                if mr > mw: mw = mr
        charwidth.update({str(chn): mw})
    bottom += 1
    width = right - left
    height = bottom - top
    print(f'width  : {left} - {right} : {width}')
    print(f'height : {top} - {bottom} : {height}')

    # find whitespace
    ws = 0
    for w in charwidth:
        ws += charwidth[w]
    ws /= len(charwidth)
    ws = int(ws / 2 + 1)
    print(f'whitespace width: {ws}')
    ws = ws + left - 1
    print(f'whitespace normalized: {ws}')
    charwidth.update({'32': ws})

    # 3 - save the new file
    f = open(f'{name}.c', 'w')
    f.write(f'#include "mgl.h"\n')
    f.write(f'\n')

    # write glyph array
    f.write(f'static const {"uint16_t" if width > 8 else "uint8_t"} _{name}_glyphs[{(lastchar - firstchar) * height}] =\n')
    f.write('{\n')
    for chn in range(firstchar, lastchar):
        f.write(f'    ')
        glyph = fontdst[str(chn)]
        for lin in range(height):
            if width > 8:
                f.write(f'0x{glyph[lin + top]>>left:04X}, ')
            else:
                f.write(f'0x{glyph[lin + top]>>left:02X}, ')
        if chn < 0x80:
            f.write(f'// {chn:02X} "{chr(chn)}"\n')
        else:
            f.write(f'// {chn:02X}"\n')
    f.write('};\n')

    f.write(f'\n')
    # write width array
    f.write(f'static const uint8_t _{name}_width[{lastchar - firstchar}] =\n')
    f.write('{\n')
    for chn in range(firstchar, lastchar):
        w = charwidth[str(chn)] - left + 1
        if w < 0: w = 0
        if chn < 0x80:
            f.write(f'    0x{w:02X}, // {chn:02X} "{chr(chn)}"\n')
        else:
            f.write(f'    0x{w:02X}, // {chn:02X}"\n')
    f.write('};\n')

    f.write(f'\n')
    # write struct
    f.write(f'const MglFont _{name} =\n')
    f.write('{\n')
    f.write(f'    .bitmap_data_horiz = (void*)_{name}_glyphs,\n')
    f.write(f'    .symbol_width = _{name}_width,\n')
    f.write(f'    .bmp_width = {width},\n')
    f.write(f'    .bmp_height = {height},\n')
    f.write(f'    .startchar = {firstchar},\n')
    f.write('};\n')

    f.write('\n')
    # f.write(f'#endif // _{name.upper()}_H\n')
    f.close()
    # '''