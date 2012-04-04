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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "iff.h"

#ifndef __inline
#define __inline
#endif
#ifndef __restrict
#define __restrict
#endif

IFFFile * iff_create()
{
    IFFFile *ptr = malloc(sizeof(IFFFile));
    if(ptr == NULL) return NULL;

    memset(ptr, 0, sizeof(IFFFile));

    return ptr;
}

int iff_read_header(IFFFile * IFFFileInfo, const uint8_t * Buffer, unsigned FileSize, char *FileName)
{
    unsigned offset;


    if(!FileSize) FileSize = ~0;
    else if(FileSize < 64)
        return 0;

    if(memcmp(Buffer, Header_IFF, 60))
        return 0;
    memcpy(IFFFileInfo->Header, Buffer, 60);
    
    IFFFileInfo->FileName = FileName;

    offset = read_uint32be(Buffer+60);
    if(offset > FileSize - 28)
        return 0;

    return 1;
}

IFFChunkNode * iff_add_chunk(IFFFile * IFFFileInfo, int Position)
{
    IFFChunkNode *ptr = malloc(sizeof(IFFChunkNode)), *node;
    if(ptr == NULL) return NULL;
    memset(ptr, 0, sizeof(IFFChunkNode));
    if(IFFFileInfo == NULL) return ptr;

    if(Position >= 0){
        node = IFFFileInfo->FirstChunk;

        if(node == NULL){
            IFFFileInfo->FirstChunk = ptr;
            IFFFileInfo->LastChunk = ptr;
        }else{
            /* Find the node we will take the place of */
            while(Position-- && node->NextChunk != NULL)
                node = node->NextChunk;

            if(node->PrevChunk == NULL)
                IFFFileInfo->FirstChunk = ptr;

            /* Shift this node and all nodes after it above us */
            ptr->PrevChunk = node->PrevChunk;
            ptr->NextChunk = node;
            node->PrevChunk = ptr;
        }
    }else{
        node = IFFFileInfo->LastChunk;

        if(node == NULL){
            IFFFileInfo->FirstChunk = ptr;
            IFFFileInfo->LastChunk = ptr;
        }else{
            /* Find the node we will take the place of */
            while(++Position && node->PrevChunk != NULL)
                node = node->PrevChunk;

            if(node->NextChunk == NULL)
                IFFFileInfo->LastChunk = ptr;

            /* Shift this node and all nodes before it below us */
            ptr->PrevChunk = node;
            ptr->NextChunk = node->NextChunk;
            node->NextChunk = ptr;
        }
    }
    
    
    IFFFileInfo->ChunkCount++;
    return ptr;
}

int iff_read_chunk(IFFChunk * ChunkInfo, const uint8_t * Buffer, unsigned MaxChunkSize)
{
    if(MaxChunkSize == 0) MaxChunkSize = ~0;

    if(MaxChunkSize < 76)
        return 0;

    memcpy(ChunkInfo->Type, Buffer+0, 4);
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
        IFFChunkNode * chunk = iff_add_chunk(IFFFileInfo, -1);
        if(chunk == NULL)
            return 0;
        if(!iff_read_chunk(&chunk->Chunk, Buffer, BufferSize)){
            free(chunk);
            return 0;
        }

        Buffer += chunk->Chunk.Size;
        BufferSize -= chunk->Chunk.Size;
    }
    return 1;
}

IFFChunk *iff_find_first_chunk(IFFFile *IFFFileInfo, const char *type, uint16_t id)
{
    IFFChunkNode *currentNode = IFFFileInfo->FirstChunk;
    do
    {
        if (type == NULL || !memcmp(type, currentNode->Chunk.Type, 4) &&
           (id   == 0    ||         id == currentNode->Chunk.ChunkID))
           return &currentNode->Chunk;
        
        currentNode = currentNode->NextChunk;
    }
    while (currentNode != IFFFileInfo->LastChunk);
    
    return NULL;
}