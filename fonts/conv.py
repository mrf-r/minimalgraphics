'''
Copyright (C) 2026 Eugene Chernyh (mrf-r)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''
# https://www.pentacom.jp/pentacom/bitfontmaker2/ can be used for glyps up to 16x16 pix
# usage in interactive mode:
#   - open python3 or idle3 from this folder
#   - from conv import FontProc
#   - fp = FontProc(whatever)
#   - fp.viewAll()
# raw arrays can be imported, as well as Bitfontmaker2 .json data
# use flip and mirror methods to fix raw import. check via viewAll()
# use shift to move particular glyphs. check via viewChar()
# 

from PIL import Image
import json
import os

MAXWIDTH = 32
MAXHEIGHT = 32

class FontProc:
    name = 'untitled'
    firstchar = 255
    glyphdata = {}
    glyphwidth = {}
    offset_width = 1
    bmpwidth = 0
    bmpheight = 0
    offset_x = 0
    offset_y = 0
    imgscale = 8
    imgcolor_fg = (0,0,0)
    imgcolor_bg = (255,255,255)
    imgcolor_grid = (200,200,150)
    imgcolor_empty = (250,200,200)

    def __init__(self, arg = None):
        # arg = sys.argv[1]
        print(os.getcwd())
        if arg != None:
            if type(arg) == str:
                self.loadJson(arg)
            elif hasattr(arg, '__iter__'):
                self.loadRaw(arg)
        else:
            print('available stuff:')
            for e in dir(self):
                if not '__' in e:
                    print(f'    {e}: {type(getattr(self, e))}')

    def loadRaw(self, data):
        print(f'data length: {len(data)}, possible bitmap size(elements): ', end='')
        for x in range(3,32):
            if len(data) % x == 0:
                print(f'{x}({int(len(data)/x)}), ', end='')
        print('')
        self.name = input('Enter name: ')
        self.firstchar = int(input('Enter first char ascii number: '))
        bmpsize = int(input('Enter bitmap size (data elements / glyph): '))
        chcount = -(-len(data) // bmpsize) # round up
        if len(data)%bmpsize != 0: print('W: data length is odd')
        for c in range(chcount):
            bmp = data[c * bmpsize : (c + 1) * bmpsize]
            gl = [0] * (16 if bmpsize < 16 else bmpsize) # BFM2 doesn't like glyphs < 16
            for i,v in enumerate(bmp): gl[i] = v
            self.glyphdata.update({str(c + self.firstchar): gl})
        if bmpsize > 16: print('W: Editing via Bitfontmaker2(pentacom.jp) is possible only for bmpsize < 16')
        self.analyzePosition()

    def loadJson(self, filename):
        print(f'processing {filename}...')
        self.name = "".join([c for c in filename.split('.')[0] if c.isalpha() or c.isdigit()]) # sorry
        print(f'name: {self.name}')
        with open(filename) as jf:
            lastchar = 0
            fontsrc = json.load(jf)
            for c in fontsrc:
                if c.isdigit():
                    chnum = int(c)
                    if chnum > lastchar: lastchar = chnum
                    if chnum < self.firstchar: self.firstchar = chnum
            glyphsize = len(fontsrc[str(lastchar)]) # MAXHEIGHT
            if self.firstchar > 32: self.firstchar = 32 # BFM2 optimizes zeroed glyphs.. do we really need space glyph?
            print(f'firstchar: {self.firstchar}')
            print(f'lastchar: {lastchar}')
            for chn in range(self.firstchar, lastchar + 1):
                if str(chn) in fontsrc:
                    self.glyphdata.update({str(chn): fontsrc[str(chn)]})
                else:
                    self.glyphdata.update({str(chn): [0] * glyphsize})
                    print(f'W: char {chn}({chr(chn)}) is not present in font')
        # self.glyphdata.update({'32': [0] * MAXHEIGHT}) # BFM2 optimizes zeroed glyphs, but we need it
        self.analyzePosition()

    def mirrorV(self):
        for chn in self.glyphdata:
            self.glyphdata[chn] = self.glyphdata[chn][::-1]
        self.analyzePosition()

    def mirrorH(self):
        for chn in self.glyphdata:
            for i, x in enumerate(self.glyphdata[chn]):
                x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1)
                x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2)
                x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4)
                x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8)
                x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16)
                self.glyphdata[chn][i] = x
        self.analyzePosition()
    
    def flipHV(self):
        maxr = 0
        for chn in self.glyphdata:
            maxr = max(maxr, max(self.glyphdata[chn]))
        bmpwidth = 32 - f'{maxr:032b}'.index('1')
        for chn in self.glyphdata:
            gl = [0] * (16 if bmpwidth < 16 else bmpwidth) # BFM2 doesn't like glyphs < 16
            for i,lin in enumerate(self.glyphdata[chn]):
                for x in range(bmpwidth):
                    gl[x] |= ((lin >> x) & 1) << i
            self.glyphdata[chn] = gl
        self.analyzePosition()

    def shiftUp(self, ch):
        if type(ch) == int: chn = ch
        elif type(ch) == str: chn = ord(ch)
        else: print(f'W: unknown arg type: {type(ch)}')
        if str(chn) in self.glyphdata:
            for i,lin in enumerate(self.glyphdata[str(chn)]):
                if i+1 < len(self.glyphdata[str(chn)]):
                    self.glyphdata[str(chn)][i] = self.glyphdata[str(chn)][i+1]
                else:
                    self.glyphdata[str(chn)][i] = 0
        else:
            print(f'W: char {chr(chn)} is not present in font')

    def shiftDown(self, ch):
        if type(ch) == int: chn = ch
        elif type(ch) == str: chn = ord(ch)
        else: print(f'W: unknown arg type: {type(ch)}')
        if str(chn) in self.glyphdata:
            prev = 0
            for i,lin in enumerate(self.glyphdata[str(chn)]):
                # for i,lin in range(MAXHEIGHT - 1, 0, -1):
                self.glyphdata[str(chn)][i] = prev
                prev = lin
        else:
            print(f'W: char {chr(chn)} is not present in font')

    def shiftLeft(self, ch):
        if type(ch) == int: chn = ch
        elif type(ch) == str: chn = ord(ch)
        else: print(f'W: unknown arg type: {type(ch)}')
        if str(chn) in self.glyphdata:
            for i,lin in enumerate(self.glyphdata[str(chn)]):
                self.glyphdata[str(chn)][i] = lin >> 1
        else:
            print(f'W: char {chr(chn)} is not present in font')

    def shiftRight(self, ch):
        if type(ch) == int: chn = ch
        elif type(ch) == str: chn = ord(ch)
        else: print(f'W: unknown arg type: {type(ch)}')
        if str(chn) in self.glyphdata:
            for i,lin in enumerate(self.glyphdata[str(chn)]):
                self.glyphdata[str(chn)][i] = lin << 1
        else:
            print(f'W: char {chr(chn)} is not present in font')

    def analyzePosition(self): # find limits of glyphs area
        left = MAXWIDTH
        right = 0
        top = MAXHEIGHT
        bottom = 0
        for chn in self.glyphdata:
            glyph = self.glyphdata[chn]
            chw = 0
            for i, lin in enumerate(glyph):
                if lin != 0:
                    if i > bottom: bottom = i
                    if i < top: top = i
                    minleft = f'{lin:032b}'[::-1].index('1')
                    if minleft < left: left = minleft
                    maxright = 32 - f'{lin:032b}'.index('1')
                    if maxright > right: right = maxright
                    chw = max(chw, maxright)
            self.glyphwidth.update({chn: chw})
        # normalize witdth for every glyph
        for chn in self.glyphwidth:
            self.glyphwidth[chn] -= left
        self.bmpheight = bottom - top + 1
        self.offset_y = top
        self.bmpwidth = right - left
        self.offset_x = left
        print(f'offset_x: {self.offset_x:2}  bmpwidth:  {self.bmpwidth:2}')
        print(f'offset_y: {self.offset_y:2}  bmpheight: {self.bmpheight:2}')
        # find whitespace
        ws = 0
        for w in self.glyphwidth:
            ws += self.glyphwidth[w]
        ws /= len(self.glyphwidth)
        ws = int(ws / 2 + 1)
        print(f'whitespace width: {ws}')
        self.glyphwidth.update({'32': ws})

    def exportC(self, filename:str = None):
        if self.bmpwidth < 8: dgt = 'uint8_t'
        elif self.bmpwidth < 16: dgt = 'uint16_t'
        elif self.bmpwidth < 32: dgt = 'uint32_t'
        else: raise ValueError(f'bitmap width is too large: {self.bmpwidth}')
        if self.bmpheight > 32: raise ValueError(f'bitmap height is too large: {self.bmpheight}')
        charcount = len(self.glyphwidth)
        if filename: name = filename
        else: name = self.name
        f = open(f'{name}.c', 'w')
        f.write(f'#include "mgl.h"\n')
        f.write(f'\n')
        # write glyph array
        f.write(f'static const {dgt} _{name}_glyphs[{charcount * self.bmpheight}] =\n')
        f.write('{\n')
        for chn in range(self.firstchar, self.firstchar + charcount): # glyphdata may be inconsecutive
            f.write(f'    ')
            glyph = self.glyphdata[str(chn)]
            for lin in range(self.bmpheight):
                if self.bmpwidth < 8: f.write(f'0x{glyph[lin + self.offset_y]>>self.offset_x:02X}, ')
                elif self.bmpwidth < 16: f.write(f'0x{glyph[lin + self.offset_y]>>self.offset_x:04X}, ')
                else: f.write(f'0x{glyph[lin + self.offset_y]>>self.offset_x:08X}, ')
            if chn < 0x80: f.write(f'// {chn:02X} "{chr(chn)}"\n')
            else: f.write(f'// {chn:02X}"\n')
        f.write('};\n')
        f.write(f'\n')
        # write width array
        f.write(f'static const uint8_t _{name}_width[{charcount}] =\n')
        f.write('{\n')
        for chn in range(self.firstchar, self.firstchar + charcount): # glyphwidth may be inconsecutive
            w = self.glyphwidth[str(chn)] + self.offset_width
            if w < 0:
                w = 0
                print(f'W: zero width: {chn}')
            if chn < 0x80: f.write(f'    0x{w:02X}, // {chn:02X} "{chr(chn)}"\n')
            else: f.write(f'    0x{w:02X}, // {chn:02X}"\n')
        f.write('};\n')
        f.write(f'\n')
        # write struct
        f.write(f'const MglFont font_{name} =\n')
        f.write('{\n')
        f.write(f'    .bitmap_data_horiz = (void*)_{name}_glyphs,\n')
        f.write(f'    .symbol_width = _{name}_width,\n')
        f.write(f'    .bmp_width = {self.bmpwidth},\n')
        f.write(f'    .bmp_height = {self.bmpheight},\n')
        f.write(f'    .startchar = {self.firstchar},\n')
        f.write(f'    .endchar = {self.firstchar + charcount - 1},\n')
        f.write('};\n')
        f.write('\n')
        f.close()

    def exportJson(self, filename:str = None):
        if filename: name = filename
        else: name = self.name
        f = open(f'{name}.json', 'w')
        f.write('{\n')
        for glyph in self.glyphdata:
            f.write(f'    "{glyph}": {self.glyphdata[glyph]},\n')
        f.write(f'    "name": "{self.name}"\n')
        f.write('}\n')
        # print(str(font).replace("'", '"'))

    def viewStr(self, string: str):
        # TODO
        pass

    def viewChar(self, ch):
        if type(ch) == int: chn = ch
        elif type(ch) == str: chn = ord(ch)
        else: print(f'W: unknown arg type: {type(ch)}')
        if str(chn) in self.glyphdata:
            print(f'{chn} ({chr(chn)}), width:{self.glyphwidth[str(chn)]}')
            for lin in self.glyphdata[str(chn)]:
                print(f'    0x{lin:08X}')
        else: print(f'W: char {chr(chn)} is not present in font')

    def viewAll(self):
        testImage = Image.new(mode='RGB', size=(16 * (self.bmpwidth + 1), 16 * (self.bmpheight + 1)), color=self.imgcolor_bg)
        for posy in range(16):
            ys = posy * (self.bmpheight + 1)
            for posx in range(16):
                xs = posx * (self.bmpwidth + 1)
                chn = posy * 16 + posx
                if str(chn) in self.glyphdata:
                    glyph = self.glyphdata[str(chn)]
                    for lin in range(self.bmpheight): # scan top to bottom
                        y = ys + lin
                        testImage.putpixel((xs,y), self.imgcolor_grid) # draw vertical left line
                        glyphline = glyph[lin + self.offset_y]
                        for col in range(self.bmpwidth):
                            x = xs + col
                            if y == ys: testImage.putpixel((x,ys), self.imgcolor_grid) # draw horizontal line
                            if (glyphline >> (col + self.offset_x)) & 1:
                                testImage.putpixel((x,y), self.imgcolor_fg)
                else: testImage.putpixel((xs,ys), self.imgcolor_grid)
        out_image = testImage.resize((testImage.width * self.imgscale, testImage.height * self.imgscale), resample=False)
        # out_image.save(f'{self.name}.png')
        out_image.show()
        testImage.close()
        out_image.close()
