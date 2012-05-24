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

#include "iff.h"

int iff_parse_rsmp(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFResourceMap *Map;
    unsigned Size = ChunkInfo->Size - 76;
    unsigned i;

    if(Size < 20)
        return 0;
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFResourceMap));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    Map = ChunkInfo->FormattedData;
    Map->Reserved = read_uint32le(Buffer);
    Map->Version = read_uint32le(Buffer+4);
    memcpy(Map->MagicNumber, Buffer+8, 4);
    Map->MagicNumber[4] = 0x00;
    Map->IFFSize = read_uint32le(Buffer+12);
    Map->TypeCount = read_uint32le(Buffer+16);
    if(Map->Reserved != 0 || Map->Version > 1)
        return 0;

    Map->ResourceTypes = calloc(Map->TypeCount, sizeof(IFFResourceType));
    if(Map->ResourceTypes == NULL)
        return 0;

    Buffer += 20; Size -= 20;
    for(i=0; i<Map->TypeCount; i++){
        IFFResourceType * Type = &Map->ResourceTypes[i];
        unsigned j;
        if(Size < 8) return 0;
        memcpy(Type->Type, Buffer, 4);
        Type->Type[4] = 0x00;
        Type->ResourceCount = read_uint32le(Buffer+4);
        Type->Resources = calloc(Type->ResourceCount, sizeof(IFFResource));
        if(Type->Resources == NULL)
            return 0;

        Buffer += 8; Size -= 8;
        for(j=0; j<Type->ResourceCount; j++){
            IFFResource * Resource = &Type->Resources[j];
            unsigned length;
            if(Size < ((Map->Version == 0) ? 9 : 11)) return 0;
            Resource->Offset = read_uint32le(Buffer);
            Buffer += 4;
            if(Map->Version == 0){
                Resource->ChunkID = read_uint16le(Buffer);
                Buffer += 2; Size -= 2;
            }else{
                Resource->ChunkID = read_uint32le(Buffer);
                Buffer += 4; Size -= 4;
            }
            Resource->Flags = read_uint16le(Buffer);
            Buffer += 2;
            Size -= 4+2;

            if(Map->Version == 0){
                /* C string */
                for(length=0; length != Size && Buffer[length]; length++);
                if(length == Size) return 0;

                if(length != 0){
                    Resource->Label = malloc(length+1);
                    if(Resource->Label == NULL) return 0;
                    strcpy(Resource->Label, (char*) Buffer);

                    Buffer += length;
                    Size   -= length;
                }
                Buffer++; Size--;
            }else{
                /* Pascal string */
                length = read_uint8le(Buffer);
                Buffer++; Size--;

                if(length != 0){
                    Resource->Label = malloc(length+1);
                    if(Resource->Label == NULL) return 0;
                    memcpy(Resource->Label, Buffer, length);
                    Resource->Label[length] = 0x00;
                }

                Buffer += length;
                Size   -= length;
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