/*
    FileHandler - General-purpose file handling library for Niotso
    refpack_dec.c - Copyright (c) 2011 Niotso Project <http://niotso.org/>
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

#include <string.h>
#include <stdint.h>

int RefPackDecompress(const uint8_t *__restrict CompressedData, size_t CompressedSize,
    uint8_t *__restrict DecompressedData, size_t DecompressedSize, unsigned HNSV){
    unsigned datawritten = 0;
    int stopflag = 0, StopFlagData;

    if(HNSV == 0) HNSV = 0xFB; /* EA default */
    if     (HNSV >= 0xFF) StopFlagData = 0x1F;
    else if(HNSV >= 0xFE) StopFlagData = 0x00;
    else if(HNSV >= 0xFD) StopFlagData = 0x01;
    else if(HNSV >= 0xFB) StopFlagData = 0x03;
    else if(HNSV >= 0xF7) StopFlagData = 0x07;
    else                  StopFlagData = 0x0F;

    while(CompressedSize > 0 && !stopflag){
        unsigned ProceedingDataLength, ReferencedDataLength, ReferencedDataOffset;
        unsigned currentbyte = *(CompressedData++);
        CompressedSize--;

        /****
        ** Fetch the opcode
        */

        /* The first byte determines the size of the entire opcode. */

        if(currentbyte <= 0x7F){ /* 2 bytes */
            if(CompressedSize < 1) return 0;
            CompressedSize -= 1;

            /* First byte */
            ProceedingDataLength = currentbyte & 0x03;
            ReferencedDataLength = ((currentbyte & 0x1C) >> 2) + 3;
            ReferencedDataOffset = (currentbyte & 0x60) << 3;

            /* Second byte */
            currentbyte = *(CompressedData++);
            ReferencedDataOffset += currentbyte;
        }else if(currentbyte <= 0xBF){ /* 3 bytes */
            if(CompressedSize < 2) return 0;
            CompressedSize -= 2;

            /* First byte */
            ReferencedDataLength = (currentbyte & 0x3F) + 4;

            /* Second byte */
            currentbyte = *(CompressedData++);
            ProceedingDataLength = (currentbyte & 0xC0) >> 6;
            ReferencedDataOffset = (currentbyte & 0x3F) << 8;

            /* Third byte */
            currentbyte = *(CompressedData++);
            ReferencedDataOffset += currentbyte;
        }else if(currentbyte <= 0xDF){ /* 4 bytes */
            if(CompressedSize < 3) return 0;
            CompressedSize -= 3;

            /* First byte */
            ProceedingDataLength = currentbyte & 0x03;
            ReferencedDataLength = ((currentbyte & 0x0C) << 6) + 5;
            ReferencedDataOffset = (currentbyte & 0x10) << 12;

            /* Second byte */
            currentbyte = *(CompressedData++);
            ReferencedDataOffset += currentbyte << 8;

            /* Third byte */
            currentbyte = *(CompressedData++);
            ReferencedDataOffset += currentbyte;

            /* Fourth byte */
            currentbyte = *(CompressedData++);
            ReferencedDataLength += currentbyte;
        }else{ /* 1 byte: Two different opcode types fall into this category */
            if(currentbyte <= HNSV){
                ProceedingDataLength = ((currentbyte & 0x1F) + 1) << 2;
            }else{
                ProceedingDataLength = currentbyte & StopFlagData;
                stopflag++;
            }
            ReferencedDataLength = 0;
        }

        /****
        ** Copy proceeding data
        */

        if(ProceedingDataLength != 0){
            if(ProceedingDataLength > CompressedSize || ProceedingDataLength > DecompressedSize)
                return 0;

            memcpy(DecompressedData, CompressedData, ProceedingDataLength);
            DecompressedSize -= ProceedingDataLength;
            CompressedSize   -= ProceedingDataLength;
            datawritten      += ProceedingDataLength;
            DecompressedData += ProceedingDataLength;
            CompressedData   += ProceedingDataLength;
        }

        /****
        ** Copy referenced data
        */

        if(ReferencedDataLength != 0){
            /* It is possible that the offset specified does not provide for a large enough buffer to copy all at once from.
               This event would be caused when the referenced data offset is set smaller than the referenced data length.
               When this occurs, the decoder is to repeatedly copy/paste the referenced data until the length is satisfied.
               We will do this in a way so that we call memcpy ceil(log2(N)) times instead of N times. */

            ReferencedDataOffset++;
            if(ReferencedDataLength > DecompressedSize || ReferencedDataOffset > datawritten)
                return 0;

            DecompressedSize -= ReferencedDataLength;
            datawritten      += ReferencedDataLength;

            if(ReferencedDataOffset == 1){
                memset(DecompressedData, *(DecompressedData-1), ReferencedDataLength);
                DecompressedData += ReferencedDataLength;
            }else{
                unsigned copylength =
                    (ReferencedDataOffset < ReferencedDataLength) ? ReferencedDataOffset : ReferencedDataLength;
                uint8_t *__restrict copysource = DecompressedData;

                memcpy(DecompressedData, DecompressedData-ReferencedDataOffset, copylength);
                DecompressedData += copylength;
                ReferencedDataLength -= copylength;

                while(ReferencedDataLength){
                    if(copylength > ReferencedDataLength)
                        copylength = ReferencedDataLength;

                    memcpy(DecompressedData, copysource, copylength);
                    DecompressedData += copylength;
                    ReferencedDataLength -= copylength;

                    if(!(copylength&0x80000000)) copylength <<= 1;
                }
            }
        }
    }

    return (stopflag && !CompressedSize && !DecompressedSize);
}