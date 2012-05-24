/*
    FileHandler - General-purpose file handling library for Niotso
    fcns.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

int iff_parse_fcns(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFConstantList *List;
    unsigned Size = ChunkInfo->Size - 76;
    unsigned i;

    if(Size < 16)
        return 0;
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFConstantList));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    List = ChunkInfo->FormattedData;
    List->Reserved = read_uint32le(Buffer);
    List->Version = read_uint32le(Buffer+4);
    memcpy(List->MagicNumber, Buffer+8, 4);
    List->MagicNumber[4] = 0x00;
    List->ConstantCount = read_uint32le(Buffer+12);
    if(List->Reserved != 0 || List->Version == 0 || List->Version > 2)
        return 0;

    List->Constants = calloc(List->ConstantCount, sizeof(IFFConstant));
    if(List->Constants == NULL)
        return 0;

    Buffer += 16; Size -= 16;
    for(i=0; i<List->ConstantCount; i++){
        IFFConstant * Constant = &List->Constants[i];
        unsigned s;
        for(s=0; s<2; s++){
            char ** string = (s==0) ? &Constant->Name : &Constant->Description;
            unsigned length;
            if(Size == 0) return 0;

            if(List->Version < 2){
                /* C string */
                for(length=0; length != Size && Buffer[length]; length++);
                if(length == Size) return 0;

                if(length != 0){
                    *string = malloc(length+1);
                    if(*string == NULL) return 0;
                    strcpy(*string, (char*) Buffer);
                }

                Buffer += length+1;
                Size   -= length+1;

                /* Skip past the 0xA3 character;
                ** see global.iff chunk 546 for why you can't do modulo-2 to detect this */
                if(Size && *Buffer == 0xA3){
                    Buffer++; Size--;
                }
            }else{
                /* Pascal string */
                length = read_uint8le(Buffer);
                Buffer++; Size--;
                if(length > 127){
                    if(Size == 0) return 0;
                    length = (length&127) | (read_uint8le(Buffer)<<7);
                    Buffer++; Size--;
                }

                if(length != 0){
                    *string = malloc(length+1);
                    if(*string == NULL) return 0;
                    memcpy(*string, Buffer, length);
                    (*string)[length] = 0x00;
                }

                Buffer += length;
                Size   -= length;
            }

            if(s == 0){
                union { float f; uint32_t v; } value;
                if(Size < 4) return 0;
                value.v = read_uint32le(Buffer);
                Constant->Value = value.f;
                Buffer+=4; Size-=4;
            }
        }
    }

    return 1;
}

void iff_free_fcns(void * FormattedData){
    IFFConstantList * List = FormattedData;
    if(List->Constants){
        unsigned c;
        for(c=0; c<List->ConstantCount; c++){
            free(List->Constants[c].Name);
            free(List->Constants[c].Description);
        }
        free(List->Constants);
    }
}