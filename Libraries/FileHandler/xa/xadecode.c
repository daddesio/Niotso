/*
    xadecode.c - Copyright (c) 2011-2012 Fatbag <X-Fi6@phppoll.org>

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
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "read_xa.h"

#ifndef write_int32
 #define write_uint32(dest, src) do {\
    (dest)[0] = ((src)&0x000000FF)>>(8*0); \
    (dest)[1] = ((src)&0x0000FF00)>>(8*1); \
    (dest)[2] = ((src)&0x00FF0000)>>(8*2); \
    (dest)[3] = ((src)&0xFF000000)>>(8*3); \
    } while(0)
 #define write_uint16(dest, src) do {\
    (dest)[0] = ((src)&0x00FF)>>(8*0); \
    (dest)[1] = ((src)&0xFF00)>>(8*1); \
    } while(0)
#endif

static FILE * hFile = NULL;
static uint8_t * XAData = NULL;
static uint8_t * WaveData = NULL;

static void Shutdown_M(const char * Message){
    fprintf(stderr, "xadecode: error: %s.\n", Message);
    free(WaveData);
    free(XAData);
    if(hFile)
        fclose(hFile);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
    int overwrite = 0;
    const char *InFile, *OutFile;
    size_t FileSize;
    xaheader_t XAHeader;
    clock_t BeginningTime, EndingTime;

    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: xadecode [-f] infile outfile\n"
        " -or-  xadecode infile\n"
        "Transcode or play a Maxis XA sound.\n"
        "Use -f to force overwriting without confirmation.\n"
        "\n"
        "Report bugs to <X-Fi6@phppoll.org>.\n"
        "xadecode is maintained by the Niotso project.\n"
        "Home page: <http://www.niotso.org/>\n");
        return 0;
    }

    if(argc >= 4 && !strcmp(argv[1], "-f")){
        overwrite++;
        InFile = argv[2];
        OutFile = argv[3];
    }else{
        InFile = argv[1];
        OutFile = argv[2];
    }

    /****
    ** Open the file and read in entire contents to memory
    */

    hFile = fopen(InFile, "rb");
    if(hFile == NULL)
        Shutdown_M("The specified input file does not exist or could not be opened for reading");
    fseek(hFile, 0, SEEK_END);
    FileSize = ftell(hFile);
    if(FileSize < 24)
        Shutdown_M("Not a valid XA file");
    fseek(hFile, 0, SEEK_SET);

    XAData = malloc(FileSize);
    if(XAData == NULL)
        Shutdown_M("Memory for this file could not be allocated");
    if(fread(XAData, 1, FileSize, hFile) != FileSize)
        Shutdown_M("The input file could not be read");
    fclose(hFile); hFile = NULL;

    /****
    ** Transcode the data from XA to LPCM
    */

    if(!xa_read_header(&XAHeader, XAData, FileSize))
        Shutdown_M("Not a valid XA file");

    if(argc >= 3){ /* Transcode */
        WaveData = malloc(44+XAHeader.dwOutSize);
        if(WaveData == NULL)
            Shutdown_M("Memory for this file could not be allocated");

        BeginningTime = clock();
        if(!xa_decode(XAData+24, WaveData+44, XAHeader.Frames, XAHeader.nChannels))
            Shutdown_M("Memory for this file could not be allocated");
        EndingTime = clock();

        free(XAData); XAData = NULL;

        /****
        ** Write the Microsoft WAV header
        */

        write_uint32(WaveData, 0x46464952); /* "RIFF" */
        write_uint32(WaveData+4, 36+XAHeader.dwOutSize);
        write_uint32(WaveData+8, 0x45564157); /* "WAVE" */
        write_uint32(WaveData+12, 0x20746d66); /* "fmt " */
        write_uint32(WaveData+16, 16);
        write_uint16(WaveData+20, 1);
        write_uint16(WaveData+22, XAHeader.nChannels);
        write_uint32(WaveData+24, XAHeader.nSamplesPerSec);
        write_uint32(WaveData+28, XAHeader.nAvgBytesPerSec);
        write_uint16(WaveData+32, XAHeader.nBlockAlign);
        write_uint16(WaveData+34, XAHeader.wBitsPerSample);
        write_uint32(WaveData+36, 0x61746164); /* "data" */
        write_uint32(WaveData+40, XAHeader.dwOutSize);

        /****
        ** Write the contents to the output file
        */

        if(!overwrite){
            hFile = fopen(OutFile, "rb");
            if(hFile != NULL){
                /* File exists */
                char c;
                fclose(hFile); hFile = NULL;
                printf("File \"%s\" exists.\nContinue anyway? (y/n) ", OutFile);
                c = getchar();
                if(c != 'y' && c != 'Y'){
                    printf("\n");
                    Shutdown_M("Aborted");
                }
            }
        }
        hFile = fopen(OutFile, "wb");
        if(hFile == NULL)
            Shutdown_M("The output file could not be opened for writing");
        printf("Extracted %u bytes in %.2f seconds.\n", (unsigned) XAHeader.dwOutSize,
            ((float)(EndingTime - BeginningTime))/CLOCKS_PER_SEC);
        fwrite(WaveData, 1, 44+XAHeader.dwOutSize, hFile);
        free(WaveData);
        fclose(hFile);
    }

    return 0;
}
