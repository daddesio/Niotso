/*
    FileHandler - General-purpose file handling library for Niotso
    FileHandler.hpp - Copyright (c) 2011-2012 Niotso Project <http://niotso.org/>
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

#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
 #define fhexport __declspec(dllexport)
#else
 #define fhexport __attribute__((visibility ("default")))
#endif

struct Asset_t {
    uint32_t Group;
    uint32_t File;
    uint32_t Type;
};

enum FErr {
    FERR_NOT_FOUND,
    FERR_OPEN,
    FERR_BLANK,
    FERR_MEMORY,
    FERR_READ,
    FERR_UNRECOGNIZED,
    FERR_INVALIDDATA
};

enum ImageFormat_t {
    FIMG_BGR24,
    FIMG_BGRA32
};

struct Image_t {
    unsigned Width, Height;
    ImageFormat_t Format;
    uint8_t * Data;
};

struct Sound_t {
    unsigned Channels;
    unsigned SamplingRate;
    unsigned BitDepth;
    unsigned Duration;
    uint8_t * Data;
};

namespace File {

inline size_t GetFileSize(FILE * hFile){
    fseek(hFile, 0, SEEK_END);
    size_t FileSize = ftell(hFile);
    fseek(hFile, 0, SEEK_SET);
    return FileSize;
}

fhexport extern int Error;
fhexport extern size_t FileSize;

fhexport uint8_t * ReadFile(const char * Filename);
fhexport Image_t * ReadImageFile(const char * Filename);
fhexport Sound_t * ReadSoundFile(const char * Filename);

}

#endif