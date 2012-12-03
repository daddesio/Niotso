/*
    FileHandler - General-purpose file handling library for Niotso
    File.cpp - Copyright (c) 2011-2012 Niotso Project <http://niotso.org/>
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

#include "FileHandler.hpp"

namespace File {

int Error = 0;
size_t FileSize = 0;

uint8_t * ReadFile(const char * Filename){
    FILE * hFile = fopen(Filename, "rb");
    if(hFile == NULL){
        File::Error = FERR_OPEN;
        return NULL;
    }

    FileSize = File::GetFileSize(hFile);
    if(FileSize == 0){
        fclose(hFile);
        File::Error = FERR_BLANK;
        return NULL;
    }

    uint8_t * InData = (uint8_t*) malloc(FileSize);
    if(InData == NULL){
        fclose(hFile);
        File::Error = FERR_MEMORY;
        return NULL;
    }

    size_t bytestransferred = fread(InData, 1, FileSize, hFile);
    fclose(hFile);
    if(bytestransferred != FileSize){
        free(InData);
        File::Error = FERR_READ;
        return NULL;
    }

    return InData;
}

}