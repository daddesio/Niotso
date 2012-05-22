/*
    FileHandler - General-purpose file handling library for Niotso
    read_utk.h - Copyright (c) 2011-2012 Niotso Project <http://niotso.org/>
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

typedef struct
{
    char     sID[4];
    uint32_t dwOutSize;
    uint32_t dwWfxSize;
    /* WAVEFORMATEX */
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint32_t cbSize;

    unsigned Frames;
    unsigned UTKDataSize;
} utkheader_t;

typedef struct {
    const uint8_t *InData;
    unsigned UnreadBitsValue, UnreadBitsCount;
    int UseLattice;
    unsigned NoiseFloor;
    float FixedCodebook[64]; /* Fixed codebook gain matrix */
    float ImpulseTrain[12];  /* Impulse train matrix */
    float R[12];             /* Autocorrelation coefficient matrix */
    float Delay[324];
    float DecompressedBlock[432];
} utkparams_t;

#ifdef __cplusplus
extern "C" {
#endif

int utk_read_header(utkheader_t * UTKHeader, const uint8_t * Buffer, unsigned FileSize);
int utk_decode(const uint8_t *__restrict InBuffer, uint8_t *__restrict OutBuffer, unsigned Frames);
void UTKGenerateTables(void);

#ifdef __cplusplus
}
#endif