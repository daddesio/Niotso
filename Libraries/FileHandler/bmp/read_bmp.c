/*
    FileHandler - General-purpose file handling library for Niotso
    read_bmp.c - Copyright (c) 2011-2012 Niotso Project <http://niotso.org/>
    Author(s): Fatbag <X-Fi6@phppoll.org>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "read_bmp.h"

#define BI_RGB  0
#define BI_RLE8 1

#ifndef read_uint32
 #define read_uint32(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint16(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
#endif

int bmp_read_header(bmpheader_t * BMPHeader, const uint8_t * Buffer, size_t FileSize){
    unsigned padding;
    if(FileSize < 54) return 0;
    BMPHeader->bfType = read_uint16(Buffer);
    BMPHeader->bfSize = read_uint32(Buffer+2);
    BMPHeader->bfReserved1 = read_uint16(Buffer+6);
    BMPHeader->bfReserved2 = read_uint16(Buffer+8);
    BMPHeader->bfOffBits = read_uint32(Buffer+10);

    BMPHeader->biSize = read_uint32(Buffer+14);
    BMPHeader->biWidth = read_uint32(Buffer+18);
    BMPHeader->biHeight = read_uint32(Buffer+22);
    BMPHeader->biPlanes = read_uint16(Buffer+26);
    BMPHeader->biBitCount = read_uint16(Buffer+28);
    BMPHeader->biCompression = read_uint32(Buffer+30);
    BMPHeader->biSizeImage = read_uint32(Buffer+34);
    BMPHeader->biXPelsPerMeter = read_uint32(Buffer+38);
    BMPHeader->biYPelsPerMeter = read_uint32(Buffer+42);
    BMPHeader->biClrUsed = read_uint32(Buffer+46);
    BMPHeader->biClrImportant = read_uint32(Buffer+50);

    if(BMPHeader->bfSize == 0) BMPHeader->bfSize = FileSize;
    BMPHeader->CompressedSize = BMPHeader->bfSize - BMPHeader->bfOffBits;
    BMPHeader->DecompressedSize = BMPHeader->biWidth * BMPHeader->biHeight * 3;
    padding = BMPHeader->biWidth%4;
    if(padding != 0) padding = (BMPHeader->biHeight-1)*(4-padding);

    if(BMPHeader->bfType != 0x4D42 ||
        BMPHeader->bfSize > FileSize ||
        BMPHeader->bfReserved1 != 0 || BMPHeader->bfReserved2 != 0 ||
        BMPHeader->biSize != 40 ||
        BMPHeader->biWidth == 0 || BMPHeader->biWidth > 4096 ||   /*< Includes negative check */
        BMPHeader->biHeight == 0 || BMPHeader->biHeight > 4096 || /*< by treating as unsigned */
        BMPHeader->biPlanes != 1 ||
        (BMPHeader->biBitCount != 24 &&
            (BMPHeader->biBitCount != 8 || BMPHeader->bfSize < 1078 /* We need room for the color palette */)) ||
        (BMPHeader->biCompression != BI_RGB &&
            (BMPHeader->biCompression != BI_RLE8 || BMPHeader->biBitCount > 8)) ||
        BMPHeader->bfOffBits >= BMPHeader->bfSize ||
        (BMPHeader->biCompression == BI_RGB &&
            ((BMPHeader->biBitCount == 24 && BMPHeader->CompressedSize < BMPHeader->DecompressedSize + padding) ||
            (BMPHeader->biBitCount == 8 && BMPHeader->CompressedSize < BMPHeader->DecompressedSize/3 + padding)))
    )   return 0;

    return 1;
}

int bmp_read_data(bmpheader_t * BMPHeader, const uint8_t *__restrict InBuffer, uint8_t *__restrict OutBuffer){
    if(BMPHeader->biBitCount == 24 && BMPHeader->biCompression == BI_RGB){
        unsigned pitch = BMPHeader->biWidth*3;
        unsigned i;
        unsigned padding = pitch%4;
        if(padding != 0) padding = 4-padding;

        for(i=0; i<BMPHeader->biHeight; i++)
            memcpy(OutBuffer + i*pitch, InBuffer+BMPHeader->bfOffBits + i*(pitch + padding), pitch);
        return 1;
    }

    if(BMPHeader->biBitCount == 32 && BMPHeader->biCompression == BI_RGB){
        unsigned i;
        for(i=0; i<BMPHeader->biHeight*BMPHeader->biWidth; i++){
            *(OutBuffer++) = *(InBuffer++);
            *(OutBuffer++) = *(InBuffer++);
            *(OutBuffer++) = *(InBuffer++);
            InBuffer++;
        }
        return 1;
    }

    if(BMPHeader->biBitCount == 8){
        const uint8_t *__restrict Palette = InBuffer + 54;
        InBuffer += BMPHeader->bfOffBits;

        if(BMPHeader->biCompression == BI_RGB){
            unsigned y, x;
            unsigned padding = BMPHeader->biWidth % 4;
            if(padding != 0) padding = 4-padding;

            for(y=0; y<BMPHeader->biHeight; y++){
                for(x=0; x<BMPHeader->biWidth; x++){
                    unsigned index = 4*(*InBuffer++);
                    *OutBuffer++ = Palette[index];
                    *OutBuffer++ = Palette[index+1];
                    *OutBuffer++ = Palette[index+2];
                }
                InBuffer += padding;
            }
            return 1;
        }

        if(BMPHeader->biCompression == BI_RLE8){
            const uint8_t *__restrict const srcend = InBuffer+BMPHeader->CompressedSize;
            uint8_t *__restrict const destend = OutBuffer+BMPHeader->DecompressedSize;

            while((unsigned)(srcend-InBuffer) >= 2){
                unsigned i;
                const unsigned command = *InBuffer++;
                const unsigned value = *InBuffer++;

                if(command == 0){
                    if(value == 0) continue; /* End of scanline reminder */
                    if(value == 1) return (InBuffer == srcend && OutBuffer == destend); /* End of bitmap reminder */
                    if(value == 2) return 0; /* Delta, used for ICO/CUR masks; wrong kind of bitmap */

                    /* Absolute copy */
                    if(value > (unsigned)(srcend-InBuffer) || value*3 > (unsigned)(destend-OutBuffer)) break;
                    for(i=0; i<value; i++){
                        unsigned index = 4*(*InBuffer++);
                        *OutBuffer++ = Palette[index];
                        *OutBuffer++ = Palette[index+1];
                        *OutBuffer++ = Palette[index+2];
                    }
                    if(value%2 && InBuffer != srcend) InBuffer++; /* Padding */
                }else{
                    /* Run */
                    unsigned index = 4*value;
                    if(command > (unsigned)(destend-OutBuffer)) break;
                    for(i=0; i<command; i++){
                        *OutBuffer++ = Palette[index];
                        *OutBuffer++ = Palette[index+1];
                        *OutBuffer++ = Palette[index+2];
                    }
                }
            }
        }
    }
    return 0;
}