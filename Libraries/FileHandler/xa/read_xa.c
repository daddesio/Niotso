/*
    FileHandler - General-purpose file handling library for Niotso
    read_xa.c - Copyright (c) 2011 Niotso Project <http://niotso.org/>
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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "read_xa.h"

#define HINIBBLE(byte) ((byte) >> 4)
#define LONIBBLE(byte) ((byte) & 0x0F)

#ifndef read_int32
 #define read_uint32(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint16(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
#endif

size_t xa_compressed_size(size_t Frames, size_t Channels)
{
    /* This function calculates the size of compressed XA data with known frames and channels, as such:
    **   Channels * (ceil(Frames/2) + ceil(Frames/28))
    **               |     a      |   |      b      |
    **
    ** a = The space required for all sample bytes in the XA data for a single channel (1 byte every 2 frames)
    ** b = The space required for all control bytes in the XA data for a single channel (1 byte every 28 frames)
    ** (a+b) is multiplied by Channels to produce the final size
    **
    ** This source package assumes a partial block at the end of the XA data is legal but a partial frame is not.
    */

    unsigned SingleChannelData = (((Frames+1)>>1) + (Frames+27)/28);

    if(Frames > UINT_MAX-27) return 0;
    if(UINT_MAX/SingleChannelData < Channels) return 0;

    return Channels*SingleChannelData;
}

int xa_read_header(xaheader_t * XAHeader, const uint8_t * Buffer, size_t FileSize)
{
    if(FileSize < 24) return 0;
    memcpy(&XAHeader->szID, Buffer, 4);
    XAHeader->dwOutSize = read_uint32(Buffer+4);
    XAHeader->wFormatTag = read_uint16(Buffer+8);
    XAHeader->nChannels = read_uint16(Buffer+10);
    XAHeader->nSamplesPerSec = read_uint32(Buffer+12);
    XAHeader->nAvgBytesPerSec = read_uint32(Buffer+16);
    XAHeader->nBlockAlign = read_uint16(Buffer+20);
    XAHeader->wBitsPerSample = read_uint16(Buffer+22);

    if(XAHeader->szID[0] != 'X' || XAHeader->szID[1] != 'A' || XAHeader->szID[3] != '\0' ||
        XAHeader->wFormatTag != 1 ||
        XAHeader->nChannels == 0 || XAHeader->nChannels > 8 ||
        XAHeader->nSamplesPerSec < 8000 || XAHeader->nSamplesPerSec > 192000 ||
            !(XAHeader->nSamplesPerSec%8000==0 || XAHeader->nSamplesPerSec%11025==0) ||
        XAHeader->wBitsPerSample != 16 ||
        XAHeader->nBlockAlign != XAHeader->nChannels*(XAHeader->wBitsPerSample>>3) ||
        XAHeader->nAvgBytesPerSec != XAHeader->nSamplesPerSec*XAHeader->nBlockAlign ||
        XAHeader->dwOutSize%XAHeader->nBlockAlign != 0
    )   return 0;

    XAHeader->Frames = XAHeader->dwOutSize/XAHeader->nBlockAlign;
    XAHeader->XADataSize = xa_compressed_size(XAHeader->Frames, XAHeader->nChannels);
    if(FileSize-24 < XAHeader->XADataSize)
        return 0;

    return 1;
}

static __inline int16_t Clip16(int sample)
{
    if(sample>=32767) return 32767;
    else if(sample<=-32768) return -32768;
    else return (int16_t) sample;
}

typedef struct {
    int PrevSample, CurSample;
    int divisor; /* residual right-shift value */
    int c1, c2;  /* predictor coefficients */
} channel_t;

static const int16_t XATable[] =
{
    0, 240,  460,  392,
    0,   0, -208, -220,
    0,   1,    3,    4,
    7,   8,   10,   11,
    0,  -1,   -3,   -4
};

int xa_decode(const uint8_t *__restrict InBuffer, uint8_t *__restrict OutBuffer, size_t Frames, size_t Channels)
{
    channel_t Channel[8];
    memset(Channel, 0, sizeof(Channel));
    if(Frames == 0) return 1;

    while(1){
        unsigned i;

        for(i=0; i<Channels; i++){
            unsigned byte      = *(InBuffer++);
            Channel[i].divisor = LONIBBLE(byte)+8;
            Channel[i].c1      = XATable[HINIBBLE(byte)];
            Channel[i].c2      = XATable[HINIBBLE(byte)+4];
        }

        for(i=0; i<14; i++){
            unsigned j;
            for(j=0; j<Channels; j++){
                unsigned byte = *(InBuffer++);
                int n;
                for(n=4; n>=0; n-=4){
                    int NewValue = byte >> n;
                    NewValue = (NewValue << 28) >> Channel[j].divisor;
                    NewValue = (NewValue + Channel[j].CurSample*Channel[j].c1 + Channel[j].PrevSample*Channel[j].c2 + 128) >> 8;
                    Channel[j].PrevSample = Channel[j].CurSample;
                    Channel[j].CurSample  = Clip16(NewValue);
                }
                *(OutBuffer++) = Channel[j].PrevSample>>(8*0);
                *(OutBuffer++) = Channel[j].PrevSample>>(8*1);
            }
            if(!--Frames) return 1;

            for(j=0; j<Channels; j++){
                *(OutBuffer++) = Channel[j].CurSample>>(8*0);
                *(OutBuffer++) = Channel[j].CurSample>>(8*1);
            }
            if(!--Frames) return 1;
        }
    }
}
