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

#include "iffparser.h"

int iff_parse_tmpl(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    IFFTemplate *Template;
    bytestream b;
    const uint8_t * ptr = Buffer;
    unsigned Size = ChunkInfo->Size;
    unsigned FieldCount;
    unsigned i;
    if(Size == 0) return 1;

    /* Walk through a first-pass to find the total field count */
    for(FieldCount=0; Size; FieldCount++){
        unsigned length = *ptr;
        if(Size < 5 || length > Size-5)
            return 0;
        ptr += length+5; Size -= length+5;
    }

    ChunkInfo->FormattedData = calloc(1, sizeof(IFFTemplate));
    if(ChunkInfo->FormattedData == NULL)
        return 0;

    Template = ChunkInfo->FormattedData;
    Template->FieldCount = FieldCount;
    Template->Fields = calloc(FieldCount, sizeof(IFFTemplateField));
    if(Template->Fields == NULL)
        return 0;

    set_bytestream(&b, Buffer, ChunkInfo->Size);

    for(i=0; i<FieldCount; i++){
        IFFTemplateField * Field = &Template->Fields[i];
        if(!read_pascal_string(&b, &Field->Name))
            return 0;

        memcpy(Field->Type, b.Buffer, 4);
        skipbytes(&b, 4);
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