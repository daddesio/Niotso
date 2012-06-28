/*
    FileHandler - General-purpose file handling library for Niotso
    rsmp.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

int iff_parse_rsmp(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFResourceMap *Map;
    bytestream b;
    unsigned i;

    if(ChunkInfo->Size < 20)
        return 0;
    set_bytestream(&b, Buffer, ChunkInfo->Size);
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFResourceMap));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    Map = ChunkInfo->FormattedData;
    Map->Reserved = read_uint32(&b);
    Map->Version = read_uint32(&b);
    memcpy(Map->MagicNumber, b.Buffer, 4);
    skipbytes(&b, 4);
    Map->IFFSize = read_uint32(&b);
    Map->TypeCount = read_uint32(&b);
    if(Map->Reserved != 0 || Map->Version > 1)
        return 0;

    Map->ResourceTypes = calloc(Map->TypeCount, sizeof(IFFResourceType));
    if(Map->ResourceTypes == NULL)
        return 0;

    for(i=0; i<Map->TypeCount; i++){
        IFFResourceType * Type = &Map->ResourceTypes[i];
        unsigned j;
        if(b.Size < 8) return 0;

        memcpy(Type->Type, b.Buffer, 4);
        skipbytes(&b, 4);
        Type->ResourceCount = read_uint32(&b);
        Type->Resources = calloc(Type->ResourceCount, sizeof(IFFResource));
        if(Type->Resources == NULL)
            return 0;

        for(j=0; j<Type->ResourceCount; j++){
            IFFResource * Resource = &Type->Resources[j];
            if(b.Size < ((Map->Version == 0) ? 9 : 11)) return 0;
            Resource->Offset = read_uint32(&b);
            Resource->ChunkID = (Map->Version == 0) ? read_uint16(&b) : read_uint32(&b);
            Resource->Flags = read_uint16(&b);

            if(Map->Version == 0){
                if(!read_c_string(&b, &Resource->Label))
                    return 0;
            }else{
                if(!read_pascal_string(&b, &Resource->Label))
                    return 0;
            }
        }
    }
    return 1;
}

void iff_free_rsmp(void * FormattedData){
    IFFResourceMap * Map = FormattedData;
    if(Map->ResourceTypes){
        unsigned t;
        for(t=0; t<Map->TypeCount; t++){
            IFFResourceType * Type = &Map->ResourceTypes[t];
            if(Type->Resources){
                unsigned r;
                for(r=0; r<Type->ResourceCount; r++)
                    free(Type->Resources[r].Label);
                free(Type->Resources);
            }
        }
        free(Map->ResourceTypes);
    }
}