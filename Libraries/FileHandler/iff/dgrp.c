/*
    FileHandler - General-purpose file handling library for Niotso
    dgrp.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#define read_split(x, y, z) \
    (largefields ? read_uint##z(x) : read_uint##y(x))

int iff_parse_dgrp(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFDrawGroup *Group;
    bytestream b;
    int largefields;
    unsigned i;

    if(ChunkInfo->Size < 52)
        return 0;
    set_bytestream(&b, Buffer, ChunkInfo->Size);
    ChunkInfo->FormattedData = calloc(1, sizeof(IFFDrawGroup));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    Group = ChunkInfo->FormattedData;
    Group->Version = read_uint16(&b);
    if(Group->Version < 20000 || Group->Version > 20004 || Group->Version == 20002)
        return 0;
    largefields = (Group->Version >= 20003);
    Group->AngleCount = read_split(&b, 16, 32);
    if(Group->AngleCount != 12)
        return 0;

    for(i=0; i<12; i++){
        IFFDrawAngle * Angle = &Group->DrawAngles[i];
        unsigned j;

        if(b.Size < ((!largefields) ? 4 : 12))
            return 0;

        if(!largefields)
            Angle->SpriteCount = read_uint16(&b);
        Angle->Direction = read_split(&b, 8, 32);
        Angle->Zoom = read_split(&b, 8, 32);
        if(largefields)
            Angle->SpriteCount = read_uint32(&b);

        if((Angle->Direction != 1 && Angle->Direction != 4 && Angle->Direction != 16 && Angle->Direction != 64)
            || (!Angle->Zoom || Angle->Zoom > 3))
            return 0;
        if(Angle->SpriteCount == 0)
            continue;

        Angle->SpriteInfo = calloc(Angle->SpriteCount, sizeof(IFFSpriteInfo));
        if(Angle->SpriteInfo == NULL)
            return 0;

        for(j=0; j<Angle->SpriteCount; j++){
            IFFSpriteInfo * Sprite = &Angle->SpriteInfo[j];
            const uint8_t size[5] = {12, 16, 0, 24, 32};

            if(b.Size < size[Group->Version - 20000])
                return 0;

            if(!largefields)
                Sprite->Type = read_uint16(&b);
            Sprite->ChunkID = read_split(&b, 16, 32);
            Sprite->SpriteIndex = read_split(&b, 16, 32);
            if(!largefields)
                Sprite->Flags = read_uint16(&b);
            Sprite->SpriteX = read_split(&b, 16, 32);
            Sprite->SpriteY = read_split(&b, 16, 32);
            if(Group->Version >= 20001){
                Sprite->ObjectZ = read_float(&b);
                if(Group->Version >= 20003){
                    Sprite->Flags = read_uint32(&b);
                    if(Group->Version == 20004){
                        Sprite->ObjectX = read_float(&b);
                        Sprite->ObjectY = read_float(&b);
                    }
                }
            }
        }
    }

    return 1;
}

void iff_free_dgrp(void * FormattedData){
    IFFDrawGroup *Group = FormattedData;
    int i;
    for(i=0; i<12; i++){
        IFFDrawAngle *Angle = &Group->DrawAngles[i];
        free(Angle->SpriteInfo);
    }
}