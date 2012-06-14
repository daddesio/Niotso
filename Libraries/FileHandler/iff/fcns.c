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

#include "iffparser.h"

int iff_parse_fcns(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFConstantList *List;
    bytestream b;
    unsigned i;

    if(ChunkInfo->Size < 16)
        return 0;
    set_bytestream(&b, Buffer, ChunkInfo->Size);
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFConstantList));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    List = ChunkInfo->FormattedData;
    List->Reserved = read_uint32(&b);
    List->Version = read_uint32(&b);
    memcpy(List->MagicNumber, b.Buffer, 4);
    skipbytes(&b, 4);
    List->ConstantCount = read_uint32(&b);
    if(List->Reserved != 0 || List->Version == 0 || List->Version > 2)
        return 0;

    List->Constants = calloc(List->ConstantCount, sizeof(IFFConstant));
    if(List->Constants == NULL)
        return 0;

    for(i=0; i<List->ConstantCount; i++){
        IFFConstant * Constant = &List->Constants[i];
        unsigned s;
        for(s=0; s<2; s++){
            char ** string = (s==0) ? &Constant->Name : &Constant->Description;
            if(List->Version < 2){
                /* C string, possible padding */
                if(!read_c_string(&b, string))
                    return 0;

                /* Skip past the 0xA3 character;
                ** see global.iff chunk 546 for why you can't do modulo-2 to detect this */
                if(b.Size && *b.Buffer == 0xA3)
                    skipbytes(&b, 1);
            }else{
                /* Extended Pascal string, no padding */
                if(!read_pascal2_string(&b, string))
                    return 0;
            }

            if(s == 0){
                if(b.Size < 4) return 0;
                Constant->Value = read_float(&b);
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