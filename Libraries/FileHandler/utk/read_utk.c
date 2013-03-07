/*
    FileHandler - General-purpose file handling library for Niotso
    read_utk.c - Copyright (c) 2011-2012 Niotso Project <http://niotso.org/>
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
#include "read_utk.h"

#ifndef read_int32
 #define read_uint32(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)) | ((x)[2]<<(8*2)) | ((x)[3]<<(8*3)))
 #define read_uint16(x) (unsigned)(((x)[0]<<(8*0)) | ((x)[1]<<(8*1)))
#endif
#ifndef write_int32
 #define write_uint16(dest, src) do {\
    (dest)[0] = ((src)&0x00FF)>>(8*0); \
    (dest)[1] = ((src)&0xFF00)>>(8*1); \
    } while(0)
#endif

#ifndef round
 #define round(x) ((x) >= 0 ? (x)+0.5 : (x)-0.5)
#endif
#ifndef clamp
 #define clamp(x, low, high) ((x) < low ? low : (x) > high ? high : (x))
#endif
#ifndef min
 #define min(x, y) ((x) < (y) ? (x) : (y))
#endif

static uint8_t ReadBits(utkcontext_t *ctx, uint8_t bits);
static void InitUTKParameters(utkcontext_t *ctx);
static void DecodeFrame(utkcontext_t *ctx);
static void GenerateExcitation(utkcontext_t *ctx, int Voiced, float * Window, int Interval);
static void Synthesize(utkcontext_t *ctx, unsigned Sample, unsigned Blocks);
static void RCtoLPC(const float *__restrict RC, float *__restrict LPC);

static const float UTKCosine[64] = {
    0,
    -.99677598476409912109375, -.99032700061798095703125, -.983879029750823974609375, -.977430999279022216796875,
    -.970982015132904052734375, -.964533984661102294921875, -.958085000514984130859375, -.9516370296478271484375,
    -.930754005908966064453125, -.904959976673126220703125, -.879167020320892333984375, -.853372991085052490234375,
    -.827579021453857421875, -.801786005496978759765625, -.775991976261138916015625, -.75019800662994384765625,
    -.724404990673065185546875, -.6986110210418701171875, -.6706349849700927734375, -.61904799938201904296875,
    -.567460000514984130859375, -.515873014926910400390625, -.4642859995365142822265625, -.4126980006694793701171875,
    -.361110985279083251953125, -.309523999691009521484375, -.257937014102935791015625, -.20634900033473968505859375,
    -.1547619998455047607421875, -.10317499935626983642578125, -.05158700048923492431640625,
    0,
    +.05158700048923492431640625, +.10317499935626983642578125, +.1547619998455047607421875, +.20634900033473968505859375,
    +.257937014102935791015625, +.309523999691009521484375, +.361110985279083251953125, +.4126980006694793701171875,
    +.4642859995365142822265625, +.515873014926910400390625, +.567460000514984130859375, +.61904799938201904296875,
    +.6706349849700927734375, +.6986110210418701171875, +.724404990673065185546875, +.75019800662994384765625,
    +.775991976261138916015625, +.801786005496978759765625, +.827579021453857421875, +.853372991085052490234375,
    +.879167020320892333984375, +.904959976673126220703125, +.930754005908966064453125, +.9516370296478271484375,
    +.958085000514984130859375, +.964533984661102294921875, +.970982015132904052734375, +.977430999279022216796875,
    +.983879029750823974609375, +.99032700061798095703125, +.99677598476409912109375
};
static const uint8_t UTKCodebook[512] = {
    4,  6,  5,  9,  4,  6,  5, 13,  4,  6,  5, 10,  4,  6,  5, 17,
    4,  6,  5,  9,  4,  6,  5, 14,  4,  6,  5, 10,  4,  6,  5, 21,
    4,  6,  5,  9,  4,  6,  5, 13,  4,  6,  5, 10,  4,  6,  5, 18,
    4,  6,  5,  9,  4,  6,  5, 14,  4,  6,  5, 10,  4,  6,  5, 25,
    4,  6,  5,  9,  4,  6,  5, 13,  4,  6,  5, 10,  4,  6,  5, 17,
    4,  6,  5,  9,  4,  6,  5, 14,  4,  6,  5, 10,  4,  6,  5, 22,
    4,  6,  5,  9,  4,  6,  5, 13,  4,  6,  5, 10,  4,  6,  5, 18,
    4,  6,  5,  9,  4,  6,  5, 14,  4,  6,  5, 10,  4,  6,  5,  0,
    4,  6,  5,  9,  4,  6,  5, 13,  4,  6,  5, 10,  4,  6,  5, 17,
    4,  6,  5,  9,  4,  6,  5, 14,  4,  6,  5, 10,  4,  6,  5, 21,
    4,  6,  5,  9,  4,  6,  5, 13,  4,  6,  5, 10,  4,  6,  5, 18,
    4,  6,  5,  9,  4,  6,  5, 14,  4,  6,  5, 10,  4,  6,  5, 26,
    4,  6,  5,  9,  4,  6,  5, 13,  4,  6,  5, 10,  4,  6,  5, 17,
    4,  6,  5,  9,  4,  6,  5, 14,  4,  6,  5, 10,  4,  6,  5, 22,
    4,  6,  5,  9,  4,  6,  5, 13,  4,  6,  5, 10,  4,  6,  5, 18,
    4,  6,  5,  9,  4,  6,  5, 14,  4,  6,  5, 10,  4,  6,  5,  2,
    4, 11,  7, 15,  4, 12,  8, 19,  4, 11,  7, 16,  4, 12,  8, 23,
    4, 11,  7, 15,  4, 12,  8, 20,  4, 11,  7, 16,  4, 12,  8, 27,
    4, 11,  7, 15,  4, 12,  8, 19,  4, 11,  7, 16,  4, 12,  8, 24,
    4, 11,  7, 15,  4, 12,  8, 20,  4, 11,  7, 16,  4, 12,  8,  1,
    4, 11,  7, 15,  4, 12,  8, 19,  4, 11,  7, 16,  4, 12,  8, 23,
    4, 11,  7, 15,  4, 12,  8, 20,  4, 11,  7, 16,  4, 12,  8, 28,
    4, 11,  7, 15,  4, 12,  8, 19,  4, 11,  7, 16,  4, 12,  8, 24,
    4, 11,  7, 15,  4, 12,  8, 20,  4, 11,  7, 16,  4, 12,  8,  3,
    4, 11,  7, 15,  4, 12,  8, 19,  4, 11,  7, 16,  4, 12,  8, 23,
    4, 11,  7, 15,  4, 12,  8, 20,  4, 11,  7, 16,  4, 12,  8, 27,
    4, 11,  7, 15,  4, 12,  8, 19,  4, 11,  7, 16,  4, 12,  8, 24,
    4, 11,  7, 15,  4, 12,  8, 20,  4, 11,  7, 16,  4, 12,  8,  1,
    4, 11,  7, 15,  4, 12,  8, 19,  4, 11,  7, 16,  4, 12,  8, 23,
    4, 11,  7, 15,  4, 12,  8, 20,  4, 11,  7, 16,  4, 12,  8, 28,
    4, 11,  7, 15,  4, 12,  8, 19,  4, 11,  7, 16,  4, 12,  8, 24,
    4, 11,  7, 15,  4, 12,  8, 20,  4, 11,  7, 16,  4, 12,  8,  3
};
static const uint8_t UTKCodeSkips[29] = {8,7,8,7,2,2,2,3,3,4,4,3,3,5,5,4,4,6,6,5,5,7,7,6,6,8,8,7,7};

int utk_read_header(utkheader_t * UTKHeader, const uint8_t * Buffer, size_t FileSize)
{
    if(FileSize < 32) return 0;
    memcpy(&UTKHeader->sID, Buffer, 4);
    UTKHeader->dwOutSize = read_uint32(Buffer+4);
    UTKHeader->dwWfxSize = read_uint32(Buffer+8);
    UTKHeader->wFormatTag = read_uint16(Buffer+12);
    UTKHeader->nChannels = read_uint16(Buffer+14);
    UTKHeader->nSamplesPerSec = read_uint32(Buffer+16);
    UTKHeader->nAvgBytesPerSec = read_uint32(Buffer+20);
    UTKHeader->nBlockAlign = read_uint16(Buffer+24);
    UTKHeader->wBitsPerSample = read_uint16(Buffer+26);
    UTKHeader->cbSize = read_uint32(Buffer+28);

    if(memcmp(UTKHeader->sID, "UTM0", 4) ||
        UTKHeader->wFormatTag != 1 ||
        UTKHeader->nChannels != 1 ||
        UTKHeader->nSamplesPerSec < 8000 || UTKHeader->nSamplesPerSec > 192000 ||
            !(UTKHeader->nSamplesPerSec%8000==0 || UTKHeader->nSamplesPerSec%11025==0) ||
        UTKHeader->wBitsPerSample != 16 ||
        UTKHeader->nBlockAlign != UTKHeader->nChannels*(UTKHeader->wBitsPerSample>>3) ||
        UTKHeader->nAvgBytesPerSec != UTKHeader->nSamplesPerSec*UTKHeader->nBlockAlign ||
        UTKHeader->dwOutSize%UTKHeader->nBlockAlign != 0 ||
        UTKHeader->cbSize != 0
    )   return 0;

    UTKHeader->Frames = UTKHeader->dwOutSize/UTKHeader->nBlockAlign;
    UTKHeader->UTKDataSize = FileSize - 32;

    return 1;
}

int utk_decode(const uint8_t *__restrict InBuffer, uint8_t *__restrict OutBuffer, size_t InSize, size_t Samples){
    utkcontext_t p;
    p.InData = InBuffer;
    p.InDataEnd = InBuffer + InSize;
    InitUTKParameters(&p);

    while(Samples){
        int i, BlockSize = min(Samples, 432);
        DecodeFrame(&p);

        for(i=0; i<BlockSize; i++){
            int value = round(p.DecompressedFrame[i]);
            value = clamp(value, -32768, 32767);
            write_uint16(OutBuffer, value);
            OutBuffer += 2;
        }
        Samples -= BlockSize;
    }
    return 1;
}

static uint8_t ReadBits(utkcontext_t *ctx, uint8_t bits){
    unsigned value = ctx->UnreadBitsValue & (255>>(8-bits));
    ctx->UnreadBitsValue >>= bits;
    ctx->UnreadBitsCount -= bits;

    if(ctx->UnreadBitsCount < 8 && ctx->InData != ctx->InDataEnd){
        ctx->UnreadBitsValue |= *(ctx->InData++) << ctx->UnreadBitsCount;
        ctx->UnreadBitsCount += 8;
    }
    return value;
}

static void InitUTKParameters(utkcontext_t *ctx){
    int i;
    float base;
    ctx->UnreadBitsValue = *(ctx->InData++);
    ctx->UnreadBitsCount = 8;
    ctx->HalvedExcitation = ReadBits(ctx, 1);
    ctx->VoicedThreshold = 32 - ReadBits(ctx, 4);
    ctx->InnovationPower[0] = (ReadBits(ctx, 4)+1) * 8; /* significand */

    base = 1.04f + (float)ReadBits(ctx, 6)/1000;
    for(i=1; i<64; i++)
        ctx->InnovationPower[i] = ctx->InnovationPower[i-1]*base;

    memset(ctx->RC, 0, 12*sizeof(float));
    memset(ctx->History, 0, 12*sizeof(float));
    memset(ctx->Delay, 0, 324*sizeof(float));
}

static void DecodeFrame(utkcontext_t *ctx){
    int i,j;
    float Excitation[118]; /* includes 5 0-valued samples to both the left and the right */
    float RCDelta[12];
    int Voiced = 0;

    memset(&Excitation[0], 0, 5*sizeof(float));
    memset(&Excitation[113], 0, 5*sizeof(float));

    for(i=0; i<12; i++){
        unsigned result = ReadBits(ctx, (i<4) ? 6 : 5);
        if(i==0 && result < ctx->VoicedThreshold) Voiced++;
        RCDelta[i] = (UTKCosine[result + ((i<4)?0:16)] - ctx->RC[i])/4;
    }

    for(i=0; i<4; i++){
        float PitchGain, InnovationGain;
        int Phase = ReadBits(ctx, 8);
        PitchGain = (float)ReadBits(ctx, 4)/15;
        InnovationGain = ctx->InnovationPower[ReadBits(ctx, 6)];

        if(!ctx->HalvedExcitation){
            GenerateExcitation(ctx, Voiced, &Excitation[5], 1);
        }else{
            /* Fill the excitation window with half as many samples and interpolate the rest */
            int Alignment = ReadBits(ctx, 1); /* whether to fill the even or odd samples */
            int FillWithZero = ReadBits(ctx, 1);
            GenerateExcitation(ctx, Voiced, &Excitation[5+Alignment], 2);

            if(FillWithZero){
                for(j=0; j<108; j+=2)
                    Excitation[5 + (1-Alignment) + j] = 0.0;
            }else{
                /* Use sinc interpolation with 6 neighboring samples */
                float *x = &Excitation[5 + (1-Alignment)];
                for(j=0; j<54; j++, x+=2)
                    *x =   (x[-1]+x[+1]) * .5973859429f
                         - (x[-3]+x[+3]) * .1145915613f
                         + (x[-5]+x[+5]) * .0180326793f;

                InnovationGain /= 2;
            }
        }

        /* If 216-Phase is negative on the first subframe, it will read into RC and History
           as the reference decoder does, which have been initialized to 0 in InitUTKParameters(). */
        for(j=0; j<108; j++)
            ctx->DecompressedFrame[108*i + j] = InnovationGain*Excitation[5+j] + PitchGain*ctx->Delay[108*i + j + (216-Phase)];
        for(j=0; j<108; j++)
            ctx->WhatIsThis[108*i + j] = PitchGain*ctx->Delay[108*i + j + (216-Phase)];
    }

    memcpy(ctx->Delay, &ctx->DecompressedFrame[108], 324*sizeof(float));

    for(i=0; i<4; i++){
        /* Linearly interpolate the reflection coefficients for the current subframe */
        for(j=0; j<12; j++)
            ctx->RC[j] += RCDelta[j];

        Synthesize(ctx, i*12, (i!=3) ? 12 : 396);
    }
}

static void GenerateExcitation(utkcontext_t *ctx, int Voiced, float * Window, int Interval){
    if(Voiced){
        int Table = 0;
        int i = 0;
        while(i<108){
            unsigned code = UTKCodebook[(Table<<8) | (ctx->UnreadBitsValue&0xFF)];
            Table = (code<2 || code>8);
            ReadBits(ctx, UTKCodeSkips[code]);

            if(code >= 4){
                /* Fill a sample with a value specified by the code; magnitude is limited to 6.0 */
                Window[i] = (code-1)/4;
                if(code&1)
                    Window[i] *= -1.0;

                i += Interval;
            }else if(code >= 2){
                /* Fill between 7 and 70 samples with 0s */
                int x = ReadBits(ctx, 6) + 7;
                x = min(x, (108 - i)/Interval);

                while(x--){
                    Window[i] = 0.0;
                    i += Interval;
                }
            }else{
                /* Fill a sample with a custom value with magnitude >= 7.0 */
                Window[i] = 7.0;
                while(ReadBits(ctx, 1))
                    Window[i]++;

                if(!ReadBits(ctx, 1))
                    Window[i] *= -1;

                i += Interval;
            }
        }
    }else{
        /* Unvoiced: restrict all samples to 0.0, -2.0, or +2.0 without using the codebook */
        int i;
        for(i=0; i<108; i+=Interval){
            if(!ReadBits(ctx, 1)) Window[i] = 0.0;
            else if(!ReadBits(ctx, 1)) Window[i] = -2.0;
            else Window[i] = 2.0;
        }
    }
}

static void Synthesize(utkcontext_t *ctx, size_t Sample, size_t Samples){
    float LPC[12];
    int offset = -1;
    RCtoLPC(ctx->RC, LPC);

    while(Samples--){
        int i;
        for(i=0; i<12; i++){
            if(++offset == 12) offset = 0;
            ctx->DecompressedFrame[Sample] += LPC[i] * ctx->History[offset];
        }
        ctx->History[offset--] = ctx->DecompressedFrame[Sample++];
    }
}

static void RCtoLPC(const float *__restrict RC, float *__restrict LPC){
    int i,j;
    float RCTemp[12], LPCTemp[12];
    RCTemp[0] = 1.0;
    memcpy(&RCTemp[1], RC, 11*sizeof(float));

    for(i=0; i<12; i++){
        LPC[i] = 0.0;
        for(j=11; j>=0; j--){
            LPC[i] -= RC[j] * RCTemp[j];
            if(j != 11)
                RCTemp[j+1] = RCTemp[j] + RC[j] * LPC[i];
        }
        RCTemp[0] = LPCTemp[i] = LPC[i];

        for(j=0; j<i; j++)
            LPC[i] -= LPCTemp[i-j-1] * LPC[j];
    }
}