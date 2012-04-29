/*
    iff.c - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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

/****
** Supported Chunks
*/

#define iff_register(x) \
    int iff_parse_##x(IFFChunk *, const uint8_t *); \
    void iff_free_##x(void *)

int iff_parse_rsmp(IFFChunk * ChunkInfo, const uint8_t * Buffer, unsigned IFFSize);
int iff_free_rsmp(void * FormattedData);

/* The order of these chunks must remain the same throughout this block: */
iff_register(bcon);
iff_register(str);
iff_register(trcn);

const char chunktypes[] =
    "STR#" "CTSS" "FAMs" "TTAs"
    "BCON"
    "TRCN"
;
int (* const iff_parse_function[])(IFFChunk*, const uint8_t*) = {
    iff_parse_str, iff_parse_str, iff_parse_str, iff_parse_str,
    iff_parse_bcon,
    iff_parse_trcn
};
void (* const iff_free_function[])(void*) = {
    iff_free_str, iff_free_str, iff_free_str, iff_free_str,
    iff_free_bcon,
    iff_free_trcn
};
/* End */


/****
** API public functions
*/

IFFFile * iff_create()
{
    IFFFile *ptr = calloc(1, sizeof(IFFFile));
    if(ptr == NULL) return NULL;
    
    ptr->Chunks = malloc(sizeof(IFFChunk));
    if(ptr->Chunks == NULL){
        free(ptr);
        return NULL;
    }
    ptr->SizeAllocated = sizeof(IFFChunk);
    return ptr;
}

int iff_read_header(IFFFile * IFFFileInfo, const uint8_t * Buffer, unsigned FileSize)
{
    unsigned offset;

    if(!FileSize) FileSize = ~0;
    else if(FileSize < 64)
        return 0;

    if(memcmp(Buffer, Header_IFF, 60))
        return 0;
    memcpy(IFFFileInfo->Header, Buffer, 60);

    offset = read_uint32be(Buffer+60);
    if(offset > FileSize - 28)
        return 0;

    return 1;
}

IFFChunk * iff_add_chunk(IFFFile * IFFFileInfo)
{
    if((IFFFileInfo->ChunkCount+1)*sizeof(IFFChunk) > IFFFileInfo->SizeAllocated){
        IFFChunk * ptr;
        if(IFFFileInfo->SizeAllocated > SIZE_MAX/2) return NULL;
        ptr = realloc(IFFFileInfo->Chunks, IFFFileInfo->SizeAllocated<<1);
        if(ptr == NULL) return NULL;
        
        IFFFileInfo->Chunks = ptr;
        IFFFileInfo->SizeAllocated<<=1;
    }

    IFFFileInfo->ChunkCount++;
    return IFFFileInfo->Chunks + IFFFileInfo->ChunkCount-1;
}

int iff_read_chunk(IFFChunk * ChunkInfo, const uint8_t * Buffer, unsigned MaxChunkSize)
{
    if(MaxChunkSize == 0) MaxChunkSize = ~0;

    if(MaxChunkSize < 76)
        return 0;

    memcpy(ChunkInfo->Type, Buffer+0, 4);
    ChunkInfo->Type[4] = 0x00;
    ChunkInfo->Size = read_uint32be(Buffer+4);
    ChunkInfo->ChunkID = read_uint16be(Buffer+8);
    ChunkInfo->Flags = read_uint16be(Buffer+10);
    memcpy(ChunkInfo->Label, Buffer+12, 64);

    if(ChunkInfo->Size < 76 || ChunkInfo->Size > MaxChunkSize)
        return 0;

    if(ChunkInfo->Size > 76){
        ChunkInfo->Data = malloc(ChunkInfo->Size - 76);
        if(ChunkInfo->Data == NULL)
            return 0;
        memcpy(ChunkInfo->Data, Buffer+76, ChunkInfo->Size - 76);
    }

    return 1;
}

int iff_enumerate_chunks(IFFFile * IFFFileInfo, const uint8_t * Buffer, unsigned BufferSize)
{
    while(BufferSize){
        IFFChunk * chunk = iff_add_chunk(IFFFileInfo);
        if(chunk == NULL)
            return 0;
        if(!iff_read_chunk(chunk, Buffer, BufferSize))
            return 0;

        Buffer += chunk->Size;
        BufferSize -= chunk->Size;
    }
    return 1;
}

int iff_parse_chunk(IFFChunk * ChunkInfo, const uint8_t * Buffer){
    unsigned i;
    for(i=0; chunktypes[i*4] != '\0'; i++){
        if(!memcmp(ChunkInfo->Type, chunktypes+i*4, 4)){
            if(iff_parse_function[i](ChunkInfo, Buffer)) return 1;
            iff_free_chunk(ChunkInfo->FormattedData);
            return 0;
        }
    }
    return 0;
}

void iff_free_chunk(IFFChunk * ChunkInfo){
    unsigned i;
    if(ChunkInfo == NULL || ChunkInfo->FormattedData) return;

    for(i=0; chunktypes[i*4] != '\0'; i++){
        if(!memcmp(ChunkInfo->Type, chunktypes+i*4, 4)){
            if(iff_free_function[i])
                iff_free_function[i](ChunkInfo->FormattedData);
            free(ChunkInfo->FormattedData);
            return;
        }
    }
}

void iff_delete(IFFFile * IFFFileInfo){
    unsigned i;
    if(IFFFileInfo == NULL) return;

    if(IFFFileInfo->Chunks != NULL){
        for(i=0; i<IFFFileInfo->ChunkCount; i++){
            iff_free_chunk(IFFFileInfo->Chunks+i);
            free(IFFFileInfo->Chunks[i].Data);
        }
        free(IFFFileInfo->Chunks);
    }

    if(IFFFileInfo->ResourceMap != NULL){
        free(IFFFileInfo->ResourceMap->Data);
        free(IFFFileInfo->ResourceMap->FormattedData);
        free(IFFFileInfo->ResourceMap);
    }

    free(IFFFileInfo);
    IFFFileInfo = NULL;
}