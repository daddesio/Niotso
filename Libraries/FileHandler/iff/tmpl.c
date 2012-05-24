/*
    FileHandler - General-purpose file handling library for Niotso
    tmpl.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

int iff_parse_tmpl(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFTemplate *Template;
    unsigned Size = ChunkInfo->Size - 76;
    unsigned FieldCount;
    unsigned i;
    const uint8_t * TempBuffer = Buffer;
    if(Size == 0) return 1;

    /* Walk through a first-pass to find the total field count */
    for(FieldCount=0; Size; FieldCount++){
        unsigned length = *TempBuffer;

        if(Size < 5 || length > Size-5)
            return 0;
        TempBuffer += length+5; Size -= length+5;
    }

    ChunkInfo->FormattedData = malloc(sizeof(IFFTemplate));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    Template = (IFFTemplate*) ChunkInfo->FormattedData;
    Template->FieldCount = FieldCount;
    Template->Fields = calloc(FieldCount, sizeof(IFFTemplateField));
    if(Template->Fields == NULL)
        return 0;

    for(i=0; i<FieldCount; i++){
        IFFTemplateField * Field = &Template->Fields[i];
        unsigned length = *Buffer;
        Field->Name = malloc(length+1);
        if(Field->Name == NULL) return 0;
        memcpy(Field->Name, Buffer+1, length);
        Field->Name[length] = 0x00;
        Buffer += length+1;

        memcpy(Field->Type, Buffer, 4);
        Buffer += 4;
    }

    return 1;
}

void iff_free_tmpl(void * FormattedData){
    IFFTemplate * Template = FormattedData;
    if(Template->Fields){
        unsigned f;
        for(f=0; f<Template->FieldCount; f++)
            free(Template->Fields[f].Name);
        free(Template->Fields);
    }
}