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

#include "iff.h"

typedef struct {
    const uint8_t * StartPos;
    const uint8_t * Buffer;
    size_t TotalSize;
    size_t Size;
    uint8_t Endian; /* 0 = little, 1 = big */
} bytestream;

static unsigned read_uint32xe(bytestream * b){
    if(b->Size >= 4){
        unsigned value = (b->Endian == 0) ? read_uint32le(b->Buffer) : read_uint32be(b->Buffer);
        b->Buffer += 4;
        b->Size -= 4;
        return value;
    }
    return 0;
}

static unsigned read_uint16xe(bytestream * b){
    if(b->Size >= 2){
        unsigned value = (b->Endian == 0) ? read_uint16le(b->Buffer) : read_uint16be(b->Buffer);
        b->Buffer += 2;
        b->Size -= 2;
        return value;
    }
    return 0;
}

static int skipbytes(bytestream * b, uint32_t bytes){
    if(b->Size < bytes) return 0;
    b->Buffer += bytes; b->Size -= bytes;
    return 1;
}

static int seekto(bytestream * b, uint32_t Position){
    if(Position > b->TotalSize) return 0;
    b->Buffer = b->StartPos + Position;
    b->Size = b->TotalSize - Position;
    return 1;
}

int iff_parse_spr(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFSpriteList *SpriteList;
    unsigned ChunkSize = ChunkInfo->Size - 76;
    bytestream b;
    unsigned i;

    b.StartPos = Buffer;
    b.Buffer = Buffer;
    b.TotalSize = ChunkSize;
    b.Size = ChunkSize;
    b.Endian = 0;

    if(ChunkSize < 12)
        return 0;
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFSpriteList));
    if(ChunkInfo->FormattedData == NULL)
        return 0;
    SpriteList = ChunkInfo->FormattedData;

    if((Buffer[0]|Buffer[1]) == 0) b.Endian++; /* Big endian */
    SpriteList->Version = read_uint32xe(&b);
    SpriteList->SpriteCount = read_uint32xe(&b);
    SpriteList->PaletteID = read_uint32xe(&b);
    if(SpriteList->Version < 502 || (SpriteList->Version > 505 && SpriteList->Version != 1001))
        return 0;
    if(SpriteList->SpriteCount == 0)
        return 1;

    SpriteList->Sprites = calloc(SpriteList->SpriteCount, sizeof(IFFSprite));
    if(SpriteList->Sprites == NULL)
        return 0;

    /* Skip past the offset table */
    if(SpriteList->SpriteCount > UINT_MAX/4 || !skipbytes(&b, SpriteList->SpriteCount*4))
        return 0;

    for(i=0; i<SpriteList->SpriteCount; i++){
        IFFSprite * Sprite = &SpriteList->Sprites[i];
        unsigned SpriteSize = 0;
        unsigned row = 0;

        if(SpriteList->Version != 1001){
            seekto(&b, 12 + 4*i);
            if(!seekto(&b, read_uint32xe(&b)))
                return 0;
            SpriteSize = b.Size;
        }else
            seekto(&b, b.Buffer - b.StartPos); /* Update b.Size */

        if(b.Size < ((SpriteList->Version == 1001) ? 18 : 10))
            return 0;
        if(SpriteList->Version == 1001){
            if(read_uint32xe(&b) != 1001)
                return 0;
            SpriteSize = read_uint32xe(&b);
            if(SpriteSize > b.Size || SpriteSize < 10)
                return 0;
        }
        Sprite->Reserved = read_uint32xe(&b);
        Sprite->Height = read_uint16xe(&b);
        Sprite->Width = read_uint16xe(&b);
        if(Sprite->Reserved != 0 || Sprite->Height == 0 || Sprite->Width == 0 || Sprite->Height > UINT_MAX/Sprite->Width/2)
            return 0;
        Sprite->IndexData = calloc(Sprite->Height*Sprite->Width, 2);
        if(Sprite->IndexData == NULL)
            return 0;

        while(1){
            /* Row command: valid commands are 0, 4, 5, and 9 */
            uint8_t Command, Count;
            if(SpriteSize < 2)
                return 0;

            Command = *(b.Buffer++);
            Count = *(b.Buffer++);
            SpriteSize -= 2;

            if(Command == 0){
                /* Do nothing */
            }else if(Command == 4){
                /* Pixel command: valid commands are 1, 2, and 3 */
                unsigned pixel = 0;

                if(row == Sprite->Height || Count < 2 || (Count -= 2) > SpriteSize || Count%2 != 0)
                    return 0;
                SpriteSize -= Count;
                while(Count){
                    uint8_t PixelCommand, PixelCount;
                    if(Count < 2)
                        return 0;

                    PixelCommand = *(b.Buffer++);
                    PixelCount = *(b.Buffer++);
                    Count -= 2;

                    if(PixelCommand == 1){
                        /* Leave pixels as transparent */
                        if(PixelCount > Sprite->Width - pixel)
                            return 0;
                        pixel += PixelCount;
                    }else if(PixelCommand == 2){
                        /* Set next n pixels to shared palette index */
                        uint8_t PaletteIndex;
                        if(PixelCount > Sprite->Width - pixel || Count < 2)
                            return 0;

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
                            return 0;
                        Count -= PixelCount + padding;

                        while(PixelCount--){
                            Sprite->IndexData[(Sprite->Width*row + pixel)*2 + 0] = *(b.Buffer++);
                            Sprite->IndexData[(Sprite->Width*row + pixel)*2 + 1] = 0xFF;
                            pixel++;
                        }
                        if(padding) b.Buffer++; /* Padding byte */
                    }else return 0;
                }
                row++;
            }else if(Command == 5){
                /* End marker */
                break;
            }else if(Command == 9){
                /* Leave rows as transparent */
                if(Count > Sprite->Height - row)
                    return 0;
                row += Count;
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
            return 0;
        }
        Sprite->BGRA32Data[4*i + 0] = Palette->Data[3*Index + 2];
        Sprite->BGRA32Data[4*i + 1] = Palette->Data[3*Index + 1];
        Sprite->BGRA32Data[4*i + 2] = Palette->Data[3*Index + 0];
        Sprite->BGRA32Data[4*i + 3] = Sprite->IndexData[2*i + 1];
    }

    return 1;
}