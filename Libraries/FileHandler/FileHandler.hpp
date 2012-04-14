/*
    FileHandler - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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
#ifndef NOWINDOWS
 #include <windows.h>
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
    FIMG_BGR24
};

struct Image_t {
    unsigned Width, Height;
    ImageFormat_t Format;
    uint8_t * Data;
};

struct Audio_t {
    unsigned Channels;
    unsigned SamplingRate;
    unsigned BitDepth;
    unsigned Duration;
    uint8_t * Data;
};

namespace File {

extern int Error;
extern size_t FileSize;

uint8_t * ReadFile(const char * Filename);
Image_t * ReadImageFile(const char * Filename);
Audio_t * ReadAudioFile(const char * Filename);

}

#endif