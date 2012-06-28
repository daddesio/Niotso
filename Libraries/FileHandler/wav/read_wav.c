/*
    FileHandler - General-purpose file handling library for Niotso
    read_wav.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include <stdint.h>
#include <string.h>
#include "read_wav.h"

#define WAVE_FORMAT_PCM 1

#ifndef read_uint32
 #define read_uint32(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint16(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
#endif

int wav_read_header(wavheader_t * WAVHeader, const uint8_t * Buffer, size_t FileSize){
    if(FileSize < 45) return 0;
    WAVHeader->sID = read_uint32(Buffer);
    WAVHeader->Size = read_uint32(Buffer+4);
    WAVHeader->DataType = read_uint32(Buffer+8);
    WAVHeader->FmtID = read_uint32(Buffer+12);
    WAVHeader->FmtSize = read_uint32(Buffer+16);
    WAVHeader->wFormatTag = read_uint16(Buffer+20);
    WAVHeader->nChannels = read_uint16(Buffer+22);
    WAVHeader->nSamplesPerSec = read_uint32(Buffer+24);
    WAVHeader->nAvgBytesPerSec = read_uint32(Buffer+28);
    WAVHeader->nBlockAlign = read_uint16(Buffer+32);
    WAVHeader->wBitsPerSample = read_uint16(Buffer+34);
    WAVHeader->DataID = read_uint32(Buffer+36);
    WAVHeader->DataSize = read_uint32(Buffer+40);

    if(WAVHeader->sID != 0x46464952 || WAVHeader->Size != FileSize-8 || WAVHeader->DataType != 0x45564157 ||
        WAVHeader->FmtID != 0x20746D66 || WAVHeader->FmtSize != 16 || WAVHeader->wFormatTag != WAVE_FORMAT_PCM ||
        WAVHeader->nChannels < 1 || WAVHeader->nChannels > 2 || WAVHeader->nSamplesPerSec == 0 ||
        (WAVHeader->nSamplesPerSec%8000 != 0 && WAVHeader->nSamplesPerSec%11025 != 0) || WAVHeader->nSamplesPerSec > 48000 ||
        WAVHeader->nAvgBytesPerSec != WAVHeader->nSamplesPerSec * WAVHeader->nBlockAlign ||
        WAVHeader->nBlockAlign != WAVHeader->nChannels * WAVHeader->wBitsPerSample >> 3 ||
        (WAVHeader->wBitsPerSample != 8 && WAVHeader->wBitsPerSample != 16) || WAVHeader->DataID != 0x61746164 ||
        WAVHeader->DataSize != FileSize - 44 || WAVHeader->DataSize % WAVHeader->nBlockAlign != 0
    )   return 0;

    return 1;
}