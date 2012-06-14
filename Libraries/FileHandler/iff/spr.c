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

#include <stdio.h>
#include "iffparser.h"

int iff_parse_spr(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFSpriteList *SpriteList;
    unsigned ChunkSize = ChunkInfo->Size;
    bytestream b;
    unsigned i;

    if(ChunkSize < 12)
        return 0;
    set_bytestream(&b, Buffer, ChunkSize);

    ChunkInfo->FormattedData = calloc(1, sizeof(IFFSpriteList));
    if(ChunkInfo->FormattedData == NULL)
        return 0;
    SpriteList = ChunkInfo->FormattedData;

    if((Buffer[0]|Buffer[1]) == 0) b.Endian++; /* Big endian */
    SpriteList->Version = read_uint32(&b);
    SpriteList->SpriteCount = read_uint32(&b);
    SpriteList->PaletteID = read_uint32(&b);
    if(SpriteList->Version < 502 || (SpriteList->Version > 505 && SpriteList->Version != 1001))
        return 0;

    if(SpriteList->Version == 1001){
        /* Sprite count is blank in version 1001, so we must walk and count up the sprites ourselves;
        ** this is easy with the sprite size field */

        /* At this point, we are looking at the first field of the first sprite */

        for(SpriteList->SpriteCount = 0; b.Size >= 18; SpriteList->SpriteCount++){
            if(read_uint32(&b) != 1001 || !skipbytes(&b, read_uint32(&b)))
                break;
        }
        seekto(&b, 12);
    }

    if(SpriteList->SpriteCount == 0)
        return 1;
    SpriteList->Sprites = calloc(SpriteList->SpriteCount, sizeof(IFFSprite));
    if(SpriteList->Sprites == NULL)
        return 0;

    for(i=0; i<SpriteList->SpriteCount; i++){
        IFFSprite * Sprite = &SpriteList->Sprites[i];
        unsigned SpriteSize = 0;
        unsigned row = 0;

        if(SpriteList->Version != 1001){
            /* Jump to the next sprite using the offset table; this is mandatory */
            seekto(&b, 12 + 4*i);
            if(!seekto(&b, read_uint32(&b)))
                return 0;
            if((SpriteSize = b.Size) < 10)
                return 0;
        }

        if(SpriteList->Version == 1001){
            seekto(&b, b.Buffer - b.StartPos); /* Resynchronize b.Size with b.Buffer */
            if(b.Size < 18)
                return 0;
            read_uint32(&b); /* Sprite version; already checked to be equal to 1001 */
            SpriteSize = read_uint32(&b);
        }

        Sprite->Reserved = read_uint32(&b);
        Sprite->Height = read_uint16(&b);
        Sprite->Width = read_uint16(&b);
        if(Sprite->Reserved != 0 || Sprite->Height == 0 || Sprite->Width == 0 || Sprite->Height > UINT_MAX/Sprite->Width/2){
            /* This happens in the third sprite of every SPR# chunk in sprites.iff */
            Sprite->InvalidDimensions = 1;
            continue;
        }
        Sprite->IndexData = calloc(Sprite->Height*Sprite->Width, 2);
        if(Sprite->IndexData == NULL)
            {printf("Error %u\n", 1);return 0;}

        while(1){
            /* Row command: valid commands are 0, 4, 5, and 9 */
            uint8_t Command, Count;
            if(SpriteSize < 2)
                {printf("Error %u\n", 2);return 0;}

            Command = *(b.Buffer++);
            Count = *(b.Buffer++);
            SpriteSize -= 2;

            if(Command == 0 || Command == 16){
                /* Start marker */
            }else if(Command == 4){
                /* Pixel command: valid commands are 1, 2, and 3 */
                unsigned pixel = 0;

                if(row == Sprite->Height || Count < 2 || (Count -= 2) > SpriteSize || Count%2 != 0)
                    {printf("Error %u\n", 3);return 0;}
                SpriteSize -= Count;
                while(Count){
                    uint8_t PixelCommand, PixelCount;
                    if(Count < 2)
                        {printf("Error %u\n", 4);return 0;}

                    PixelCommand = *(b.Buffer++);
                    PixelCount = *(b.Buffer++);
                    Count -= 2;

                    if(PixelCommand == 1){
                        /* Leave next n pixels as transparent */
                        if(PixelCount > Sprite->Width - pixel)
                            {printf("Error %u\n", 5);return 0;}
                        pixel += PixelCount;
                    }else if(PixelCommand == 2){
                        /* Set next n pixels to shared palette index */
                        uint8_t PaletteIndex;
                        if(PixelCount > Sprite->Width - pixel || Count < 2)
                            {printf("Error %u\n", 6);return 0;}

                        PaletteIndex = *(b.Buffer++);
                        b.Buffer++; /* Padding byte */
                        Count -= 2;

                        while(PixelCount--){
                            Sprite->IndexData[(Sprite->Width*row + pixel)*2 + 0] = PaletteIndex;
                            Sprite->IndexData[(Sprite->Width*row + pixel)*2 + 1] = 0xFF;
                            pixel++;
                        }
                    }else if(PixelCommand == 3){
                        /* Set next n pixels to n palette indices */
                        int padding = PixelCount%2;
                        if(PixelCount > Sprite->Width - pixel || PixelCount + padding > Count)
                            {printf("Error %u\n", 7);return 0;}
                        Count -= PixelCount + padding;

                        while(PixelCount--){
                            Sprite->IndexData[(Sprite->Width*row + pixel)*2 + 0] = *(b.Buffer++);
                            Sprite->IndexData[(Sprite->Width*row + pixel)*2 + 1] = 0xFF;
                            pixel++;
                        }
                        if(padding) b.Buffer++; /* Padding byte */
                    }else {printf("Error %u\n", 8);return 0;}
                }
                row++;
            }else if(Command == 5){
                /* End marker */
                break;
            }else if(Command == 9){
                /* Leave rows as transparent */
                if(Count > Sprite->Height - row)
                    {printf("Error %u\n", 9);return 0;}
                row += Count;
            }else {printf("Error %u\n", 10);return 0;}
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