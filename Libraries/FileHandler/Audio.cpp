/*
    FileHandler - General-purpose file handling library for Niotso
    Audio.cpp - Copyright (c) 2011-2012 Niotso Project <http://niotso.org/>
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
#include <mpg123.h>
#include "wav/read_wav.h"
#include "xa/read_xa.h"
#include "utk/read_utk.h"

namespace File {

enum SoundType {
    FSND_WAV,
    FSND_XA,
    FSND_UTK,
    FSND_MP3,
    FSND_COUNT
};

static uint8_t * ReadWAV(Sound_t * Sound, const uint8_t * InData, size_t FileSize);
static uint8_t * ReadXA(Sound_t * Sound, const uint8_t * InData, size_t FileSize);
static uint8_t * ReadUTK(Sound_t * Sound, const uint8_t * InData, size_t FileSize);
static uint8_t * ReadMP3(Sound_t * Sound, const uint8_t * InData, size_t FileSize);

static const uint8_t Signature[] = {
    'R', //WAV
    'X', //XA
    'U', //UTK
    0xFF //MP3
};
static uint8_t* (* const SoundFunction[])(Sound_t*, const uint8_t*, size_t) = {
    ReadWAV,
    ReadXA,
    ReadUTK,
    ReadMP3
};

Sound_t * ReadSoundFile(const char * Filename){
    uint8_t * InData = File::ReadFile(Filename);
    if(InData == NULL) return NULL;

    if(File::FileSize < 4){
        free(InData);
        File::Error = FERR_INVALIDDATA;
        return NULL;
    }

    Sound_t * Sound = (Sound_t*) malloc(sizeof(Sound_t));
    if(Sound == NULL){
        free(InData);
        File::Error = FERR_MEMORY;
        return NULL;
    }

    for(int i=0; i<FSND_COUNT; i++){
        if(InData[0] == Signature[i]){
            uint8_t * OutData = SoundFunction[i](Sound, InData, File::FileSize);
            free(InData);
            if(OutData == NULL){
                File::Error = FERR_INVALIDDATA;
                return NULL;
            }
            return Sound;
        }
    }

    free(InData);
    File::Error = FERR_UNRECOGNIZED;
    return NULL;
}

static uint8_t * ReadWAV(Sound_t * Sound, const uint8_t * InData, size_t FileSize){
    wavheader_t WAVHeader;
    if(!wav_read_header(&WAVHeader, InData, FileSize)){
        return NULL;
    }

    uint8_t * OutData = (uint8_t*) malloc(WAVHeader.DataSize);
    if(OutData == NULL){
        return NULL;
    }
    memcpy(OutData, InData+44, WAVHeader.DataSize);

    Sound->Channels = WAVHeader.nChannels;
    Sound->SamplingRate = WAVHeader.nSamplesPerSec;
    Sound->BitDepth = WAVHeader.wBitsPerSample;
    Sound->Duration = WAVHeader.DataSize / WAVHeader.nBlockAlign;
    Sound->Data = OutData;
    return OutData;
}


static uint8_t * ReadXA(Sound_t * Sound, const uint8_t * InData, size_t FileSize){
    xaheader_t XAHeader;
    if(!xa_read_header(&XAHeader, InData, FileSize)){
        return NULL;
    }

    uint8_t * OutData = (uint8_t*) malloc(XAHeader.dwOutSize);
    if(OutData == NULL){
        return NULL;
    }
    if(!xa_decode(InData+24, OutData, XAHeader.Frames, XAHeader.nChannels)){
        free(OutData);
        return NULL;
    }

    Sound->Channels = XAHeader.nChannels;
    Sound->SamplingRate = XAHeader.nSamplesPerSec;
    Sound->BitDepth = XAHeader.wBitsPerSample;
    Sound->Duration = XAHeader.dwOutSize / XAHeader.nBlockAlign;
    Sound->Data = OutData;
    return OutData;
}

static uint8_t * ReadUTK(Sound_t * Sound, const uint8_t * InData, size_t FileSize){
    utkheader_t UTKHeader;
    if(!utk_read_header(&UTKHeader, InData, FileSize)){
        return NULL;
    }

    uint8_t * OutData = (uint8_t*) malloc(UTKHeader.dwOutSize);
    if(OutData == NULL){
        return NULL;
    }

    if(!utk_decode(InData+32, OutData, FileSize-32, UTKHeader.Frames)){
        free(OutData);
        return NULL;
    }

    Sound->Channels = 1;
    Sound->SamplingRate = UTKHeader.nSamplesPerSec;
    Sound->BitDepth = UTKHeader.wBitsPerSample;
    Sound->Duration = UTKHeader.dwOutSize / UTKHeader.nBlockAlign;
    Sound->Data = OutData;
    return OutData;
}

static uint8_t * ReadMP3(Sound_t * Sound, const uint8_t * InData, size_t FileSize){
    mpg123_handle *mh;
    if(mpg123_init() != MPG123_OK || (mh = mpg123_new(NULL, NULL)) == NULL){
        mpg123_exit();
        return NULL;
    }

    long rate;
    int channels, encoding;
    unsigned samples;
    size_t OutSize;
    uint8_t * OutData;

    if(mpg123_format_none(mh) != MPG123_OK ||
        mpg123_format(mh, 44100, MPG123_MONO | MPG123_STEREO, MPG123_ENC_SIGNED_16) != MPG123_OK ||
        mpg123_open_feed(mh) != MPG123_OK ||
        mpg123_feed(mh, InData, FileSize) != MPG123_OK ||
        mpg123_set_filesize(mh, FileSize) != MPG123_OK ||
        mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK ||
        (samples = mpg123_length(mh)) == 0 ||
        (OutData = (uint8_t*) malloc(OutSize = samples * channels * 2)) == NULL
    ){
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return NULL;
    }

    size_t decoded;
    mpg123_read(mh, OutData, OutSize, &decoded);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    if(decoded != OutSize){
        free(OutData);
        return NULL;
    }

    Sound->Channels = channels;
    Sound->SamplingRate = rate;
    Sound->BitDepth = 16;
    Sound->Duration = samples;
    Sound->Data = OutData;
    return OutData;
}

}