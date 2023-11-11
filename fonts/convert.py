# convert font json form https://www.pentacom.jp/pentacom/bitfontmaker2/ to this library
# will not change "true" to "True"

from PIL import Image
import json
import sys
import os

os.get

for arg in sys.argv:
    print(f'processing {arg}...')
    fontsrc = json.load(arg)
    fontdst = {}
    # 1 - normalize charcount to basic ascii
    blank_chars = 0
    for chn in range(0x20, 0x7F):
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
    for chn in range(0x20, 0x7F):
        glyph = fontdst[str(chn)]
        minpass = False
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
                charwidth.update({str(chn): mr})
    print(f'width  : {left} - {right} : {right - left}')
    print(f'height : {top} - {bottom} : {bottom - top}')
    # 3 - save the new file
    f = open(f'{arg}.h', 'w')
    f.write('#ifndef ')
    for chn in range(0x20, 0x7F):
        glyph = fontdst[str(chn)]
        nglyph = glyph