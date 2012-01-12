/*
    read_utk.h - Copyright (c) 2011 Fatbag <X-Fi6@phppoll.org>

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
    char  sID[4];
    DWORD dwOutSize;
    DWORD dwWfxSize;
    /* WAVEFORMATEX */
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    DWORD cbSize;

    unsigned Frames;
    unsigned UTKDataSize;
} utkheader_t;

typedef struct {
	const uint8_t *InData;
	unsigned x, y;
	unsigned a, b;
	float c[88];
	float d[324];
	float DecompressedBlock[432];
} utkparams_t;

#ifdef __cplusplus
extern "C" {
#endif

int utk_read_header(utkheader_t * UTKHeader, const uint8_t * Buffer, unsigned FileSize);
int utk_decode(const uint8_t *__restrict InBuffer, uint8_t *__restrict OutBuffer, unsigned Frames);
void UTKGenerateTables(void);
uint8_t ReadCC(utkparams_t *p, uint8_t i);
void SetUTKParameters(utkparams_t *p);
void DecompressBlock(utkparams_t *p);
void Unknown1(utkparams_t *p, int Branch, float * Window, int Interval);
void Unknown2(utkparams_t *p, unsigned Sample, unsigned Blocks);
void Unknown2_1(float *__restrict c64, float *__restrict Matrix);

#ifdef __cplusplus
}
#endif