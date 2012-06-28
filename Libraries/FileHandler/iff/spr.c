/*
    FileHandler - General-purpose file handling library for Niotso
    spr.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

int iff_parse_spr(IFFChunk * ChunkInfo, const uint8_t * Buffer){
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
    SpriteList->SpriteCount = read_uint32(&b);
    SpriteList->PaletteID = read_uint32(&b);
    if(SpriteList->Version < 502 || (SpriteList->Version > 505 && SpriteList->Version != 1001))
        return 0;

    if(SpriteList->Version != 1001){
        if(SpriteList->SpriteCount > b.Size/4)
            return 0;
    }else{
        /* Sprite count is blank in version 1001, so we must walk and count up the sprites ourselves;
        ** this is easy with the sprite size field */
        for(SpriteList->SpriteCount = 0; b.Size >= 16; SpriteList->SpriteCount++){
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

        if(SpriteList->Version != 1001){
            /* Jump to the next sprite using the offset table; this is mandatory */
            seekto(&b, 12 + 4*i);
            if(!seekto(&b, read_uint32(&b)) || b.Size < 8)
                return 0;
            SpriteSize = b.Size;
        }else{
            /* Jump to the next sprite using the sprite size field; this is mandatory */
            seekto(&b, NextSpriteOffset);
            SpriteSize = read_uint32(&b);
            NextSpriteOffset += SpriteSize + 8;
        }

        Sprite->Reserved = read_uint32(&b);
        Sprite->Height = read_uint16(&b);
        Sprite->Width = read_uint16(&b);
        if(Sprite->Reserved != 0 || Sprite->Height == 0 || Sprite->Width == 0 || Sprite->Height > UINT_MAX/2/Sprite->Width){
            /* This happens in the third sprite of every SPR# chunk in sprites.iff */
            Sprite->InvalidDimensions = 1;
            continue;
        }

        SpriteSize -= 8;

        Sprite->IndexData = calloc(Sprite->Width*Sprite->Height, 2);
        if(Sprite->IndexData == NULL)
            return 0;

        while(1){
            /****
            ** Row command: valid commands are 0, 4, 5, 9, and 16
            */

            uint8_t RowCommand, RowCount;

            if(SpriteSize < 2)
                return 0;
            RowCommand = *(b.Buffer++);
            RowCount = *(b.Buffer++);
            SpriteSize -= 2;

            if(RowCommand == 0 || RowCommand == 16){
                /* Start marker */
            }else if(RowCommand == 4){
                /****
                ** Pixel command: valid commands are 1, 2, and 3
                */

                unsigned pixel = 0;

                if(row == Sprite->Height || RowCount < 2 || (RowCount -= 2) > SpriteSize || RowCount%2 != 0)
                    return 0;
                SpriteSize -= RowCount;

                while(RowCount){
                    uint8_t PixelCommand, PixelCount;
                    uint8_t * IndexData;
                    if(RowCount < 2)
                        return 0;

                    PixelCommand = *(b.Buffer++);
                    PixelCount = *(b.Buffer++);
                    RowCount -= 2;

                    if(PixelCount > Sprite->Width - pixel)
                        return 0;

                    IndexData = Sprite->IndexData + (Sprite->Width*row + pixel)*2;
                    pixel += PixelCount;

                    if(PixelCommand == 1){
                        /* Leave next n pixels as transparent */
                    }else if(PixelCommand == 2){
                        /* Set next n pixels to shared palette index */

                        uint8_t PaletteIndex;
                        if(RowCount < 2)
                            return 0;

                        PaletteIndex = *(b.Buffer++);
                        b.Buffer++; /* Padding byte */
                        RowCount -= 2;

                        while(PixelCount--){
                            *IndexData++ = PaletteIndex;
                            *IndexData++ = 0xFF;
                        }
                    }else if(PixelCommand == 3){
                        /* Set next n pixels to n palette indices */

                        int padding = PixelCount%2;
                        if(PixelCount + padding > RowCount)
                            return 0;
                        RowCount -= PixelCount + padding;

                        while(PixelCount--){
                            *IndexData++ = *(b.Buffer++);
                            *IndexData++ = 0xFF;
                        }
                        if(padding) b.Buffer++; /* Padding byte */
                    }else return 0;
                }
                row++;
            }else if(RowCommand == 5){
                /* End marker */
                break;
            }else if(RowCommand == 9){
                /* Leave rows as transparent */

                if(RowCount > Sprite->Height - row)
                    return 0;
                row += RowCount;
            }else return 0;
        }
    }

    return 1;
}

void iff_free_spr(void * FormattedData){
    IFFSpriteList *SpriteList = FormattedData;
    if(SpriteList->Sprites){
        unsigned s;
        for(s=0; s<SpriteList->SpriteCount; s++){
            IFFSprite *Sprite = &SpriteList->Sprites[s];
            free(Sprite->IndexData);
            free(Sprite->BGRA32Data);
            free(Sprite->ZBuffer);
        }
        free(SpriteList->Sprites);
    }
}

int iff_depalette(IFFSprite * Sprite, const IFFPalette * Palette){
    unsigned PixelCount = Sprite->Width*Sprite->Height;
    unsigned i;

    Sprite->BGRA32Data = malloc(PixelCount*4);
    if(Sprite->BGRA32Data == NULL) return 0;

    for(i=0; i<PixelCount; i++){
        uint8_t Index = Sprite->IndexData[2*i + 0];
        if(Index >= Palette->ColorCount){
            free(Sprite->BGRA32Data);
            Sprite->BGRA32Data = NULL;
            return 0;
        }
        Sprite->BGRA32Data[4*i + 0] = Palette->Data[3*Index + 2];
        Sprite->BGRA32Data[4*i + 1] = Palette->Data[3*Index + 1];
        Sprite->BGRA32Data[4*i + 2] = Palette->Data[3*Index + 0];
        Sprite->BGRA32Data[4*i + 3] = Sprite->IndexData[2*i + 1];
    }

    return 1;
}