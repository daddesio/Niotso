/*
    FileHandler - General-purpose file handling library for Niotso
    spr2.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include "iffparser.h"

int iff_parse_spr2(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFSpriteList *SpriteList;
    bytestream b;
    unsigned NextSpriteOffset; /* Used for Version 1001 in place of the offset table */
    unsigned i;

    if(ChunkInfo->Size < 12)
        return 0;
    set_bytestream(&b, Buffer, ChunkInfo->Size);
    if((Buffer[0]|Buffer[1]) == 0) b.Endian++; /* Big endian */

    ChunkInfo->FormattedData = calloc(1, sizeof(IFFSpriteList));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    SpriteList = ChunkInfo->FormattedData;
    SpriteList->Version = read_uint32(&b);
    if(SpriteList->Version != 1000 && SpriteList->Version != 1001)
        return 0;

    if(SpriteList->Version == 1000){
        SpriteList->SpriteCount = read_uint32(&b);
        SpriteList->PaletteID = read_uint32(&b);
        if(SpriteList->SpriteCount > b.Size/4)
            return 0;
    }else{
        SpriteList->PaletteID = read_uint32(&b);
        skipbytes(&b, 4);

        /* Sprite count is blank in version 1001, so we must walk and count up the sprites ourselves;
        ** this is easy with the sprite size field */
        for(SpriteList->SpriteCount = 0; b.Size >= 24; SpriteList->SpriteCount++){
            if(read_uint32(&b) != 1001 || !skipbytes(&b, read_uint32(&b)))
                return 0;
        }
        NextSpriteOffset = 16;
    }

    if(SpriteList->SpriteCount == 0)
        return 1;
    SpriteList->Sprites = calloc(SpriteList->SpriteCount, sizeof(IFFSprite));
    if(SpriteList->Sprites == NULL)
        return 0;

    for(i=0; i<SpriteList->SpriteCount; i++){
        IFFSprite * Sprite = &SpriteList->Sprites[i];
        unsigned SpriteSize;
        unsigned row = 0;
        int j;

        if(SpriteList->Version != 1001){
            /* Jump to the next sprite using the offset table; this is mandatory */
            seekto(&b, 12 + 4*i);
            if(!seekto(&b, read_uint32(&b)) || b.Size < 16)
                return 0;
            SpriteSize = b.Size;
        }else{
            /* Jump to the next sprite using the sprite size field; this is mandatory */
            seekto(&b, NextSpriteOffset);
            SpriteSize = read_uint32(&b);
            NextSpriteOffset += SpriteSize + 8;
        }

        Sprite->Width = read_uint16(&b);
        Sprite->Height = read_uint16(&b);
        Sprite->Flags = read_uint32(&b);
        Sprite->PaletteID = read_uint16(&b);
        Sprite->TransparentColor = read_uint16(&b);
        Sprite->YLoc = read_uint16(&b);
        Sprite->XLoc = read_uint16(&b);
        if((Sprite->Flags != 1 && Sprite->Flags != 3 && Sprite->Flags != 7) || Sprite->TransparentColor >= 256)
            return 0;
        if(Sprite->Height == 0 || Sprite->Width == 0 || Sprite->Height > UINT_MAX/2/Sprite->Width){
            /* This happens in the many chunks in 2personportal.spf */
            Sprite->InvalidDimensions = 1;
            continue;
        }

        SpriteSize -= 16;

        Sprite->IndexData = malloc(Sprite->Width*Sprite->Height*2);
        if(Sprite->IndexData == NULL)
            return 0;
        for(j=0; j<Sprite->Width*Sprite->Height; j++){
            Sprite->IndexData[2*j + 0] = Sprite->TransparentColor;
            Sprite->IndexData[2*j + 1] = 0x00;
        }

        if(Sprite->Flags >= 3){ /* Has the Z-Buffer flag */
            Sprite->ZBuffer = malloc(Sprite->Width*Sprite->Height);
            if(Sprite->ZBuffer == NULL)
                return 0;
            memset(Sprite->ZBuffer, 0xFF, Sprite->Width*Sprite->Height);
        }

        while(1){
            /****
            ** Row command: valid commands are 0, 4, and 5
            */

            uint8_t RowCommand; /* 3 bits */
            uint16_t RowCount;  /* 13 bits */

            if(SpriteSize < 2)
                return 0;
            RowCount = read_uint16le(b.Buffer);
            RowCommand = RowCount >> 13;
            RowCount &= 0x1FFF;

            b.Buffer += 2;
            SpriteSize -= 2;

            if(RowCommand == 0){
                /****
                ** Pixel command: valid commands are 1, 2, 3, and 6
                */

                unsigned pixel = 0;

                if(row == Sprite->Height || RowCount < 2 || (RowCount -= 2) > SpriteSize || RowCount%2 != 0)
                    return 0;
                SpriteSize -= RowCount;

                while(RowCount){
                    uint8_t PixelCommand; /* 3 bits */
                    uint16_t PixelCount;  /* 13 bits */
                    uint8_t * IndexData, * ZBuffer;
                    if(RowCount < 2)
                        return 0;

                    PixelCount = read_uint16le(b.Buffer);
                    PixelCommand = PixelCount >> 13;
                    PixelCount &= 0x1FFF;

                    b.Buffer += 2;
                    RowCount -= 2;

                    if(PixelCount > Sprite->Width - pixel)
                        return 0;

                    IndexData = Sprite->IndexData + (Sprite->Width*row + pixel)*2;
                    ZBuffer = Sprite->ZBuffer + (Sprite->Width*row + pixel)*1;
                    pixel += PixelCount;

                    if(PixelCommand == 1){
                        /* color+z-buffer: Set next n pixels to n palette indices */

                        if(Sprite->Flags < 3 || PixelCount*2 > RowCount)
                            return 0;
                        RowCount -= PixelCount*2;

                        while(PixelCount--){
                            *ZBuffer++ = *(b.Buffer++);
                            IndexData[0] = *(b.Buffer++);
                            IndexData[1] = (IndexData[0] != Sprite->TransparentColor) ? 0xFF : 0x00;
                            IndexData += 2;
                        }
                    }else if(PixelCommand == 2){
                        /* color+z-buffer+alpha: Set next n pixels to n palette indices */

                        int padding = PixelCount%2;
                        if(Sprite->Flags < 7 || PixelCount*3 + padding > RowCount)
                            return 0;
                        RowCount -= PixelCount*3 + padding;

                        while(PixelCount--){
                            *ZBuffer++ = *(b.Buffer++);
                            *IndexData++ = *(b.Buffer++);
                            *IndexData++ = *(b.Buffer++);
                        }
                        if(padding) b.Buffer++; /* Padding byte */
                    }else if(PixelCommand == 3){
                        /* Leave next n pixels as transparent */
                    }else if(PixelCommand == 6){
                        /* color: Set next n pixels to n palette indices */

                        int padding = PixelCount%2;
                        if(PixelCount + padding > RowCount)
                            return 0;
                        RowCount -= PixelCount + padding;

                        while(PixelCount--){
                            IndexData[0] = *(b.Buffer++);
                            IndexData[1] = (IndexData[0] != Sprite->TransparentColor) ? 0xFF : 0x00;
                            if(Sprite->Flags >= 3)
                                *ZBuffer++ = (IndexData[0] != Sprite->TransparentColor) ? 0x00 : 0xFF;
                            IndexData += 2;
                        }
                        if(padding) b.Buffer++; /* Padding byte */
                    } else return 0;
                }
                row++;
            }else if(RowCommand == 4){
                /* Leave rows as transparent */

                if(RowCount > Sprite->Height - row)
                    return 0;
                row += RowCount;
            }else if(RowCommand == 5){
                /* End marker */
                break;
            }else return 0;
        }
    }

    return 1;
}