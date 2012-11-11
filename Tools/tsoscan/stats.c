/*
    tsoscan - IFF statistical webpage generator
    stats.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Ahmed El-Mahdawy <aa.mahdawy.10@gmail.com>

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
#include <iff/iff.h>
#include "tsoscan.h"

int stats_create(IFFStats *stats){
    stats->AverageChunkCount = -1;
    return 0;
}

int stats_version_increment(IFFStats *stats, char *type, unsigned version){
    ChunkStats *chunk = NULL;
    VersionInfo *verinfo = NULL;
    unsigned i;

    if(stats == NULL) return 0;
    if(type == NULL) return 0;

    for(i=0; i<stats->ChunkTypeCount; i++){
        if(!strcmp(stats->ChunkTypes[i].Type, type)){
            chunk = stats->ChunkTypes+i;
            if(chunk == NULL) return 0;
        }
    }
    if(chunk == NULL){
        stats->ChunkTypes = realloc(stats->ChunkTypes, ++(stats->ChunkTypeCount)*sizeof(ChunkStats));
        if(stats->ChunkTypes == NULL) return 0;
        chunk = stats->ChunkTypes + stats->ChunkTypeCount - 1;
        memset(chunk, 0, sizeof(ChunkStats));
        strcpy(chunk->Type, type);
    }
    chunk->ChunkCount++;

    for(i=0; i<chunk->VersionCount; i++){
        if(chunk->Versions[i].Version == version){
            verinfo = chunk->Versions+i;
            if(verinfo == NULL) return 0;
        }
    }
    if(verinfo == NULL){
        chunk->Versions = realloc(chunk->Versions, ++(chunk->VersionCount)*sizeof(VersionInfo));
        if(chunk->Versions == NULL) return 0;
        verinfo = chunk->Versions + chunk->VersionCount - 1;
        memset(verinfo, 0, sizeof(VersionInfo));
        verinfo->Version = version;
    }
    verinfo->Count++;

    return 1;
}

unsigned stats_get_version(char *type, uint8_t *data){
    /* I could've used iff_parse_chunk instead to determine chunk versions, but this
       would be unnecessarily slow and would require all chunk parsers to be written,
       defeating the purpose of this tool. */

    if(!strcmp(type, "STR#") || !strcmp(type, "CTSS") || !strcmp(type, "FAMs") ||
       !strcmp(type, "TTAs") || !strcmp(type, "CST")  || !strcmp(type, "BHAV") ||
       !strcmp(type, "DGRP") || !strcmp(type, "POSI"))
        return read_uint16le(data);

    if(!strcmp(type, "FCNS") || !strcmp(type, "OBJf") || !strcmp(type, "Optn") ||
       !strcmp(type, "Rcon") || !strcmp(type, "TPRP") || !strcmp(type, "SLOT") ||
       !strcmp(type, "TRCN") || !strcmp(type, "rsmp"))
        return read_uint32le(data+4);

    if(!strcmp(type, "OBJD") || !strcmp(type, "PALT") || !strcmp(type, "SPR2"))
        return read_uint32le(data);

    if(!strcmp(type, "TTAB"))
        return read_uint16le(data+2);

    if(!strcmp(type, "SPR#")){
        if(data[0] == 0) return read_uint32be(data);
        else return read_uint32le(data);
    }

    return -1;
}

void stats_delete(IFFStats *stats){
    unsigned i;

    if(stats == NULL) return;
    if(stats->ChunkTypes != NULL){
        for(i=0; i<stats->ChunkTypeCount; i++)
            free(stats->ChunkTypes[i].Versions);
        free(stats->ChunkTypes);
    }
}