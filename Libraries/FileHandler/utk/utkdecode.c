/*
    utkdecode.c - Copyright (c) 2011-2012 Fatbag <X-Fi6@phppoll.org>

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
#include "read_utk.h"

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
static uint8_t * UTKData = NULL;
static uint8_t * WaveData = NULL;

static void Shutdown_M(const char * Message){
    fprintf(stderr, "utkdecode: error: %s.\n", Message);
    free(WaveData);
    free(UTKData);
    if(hFile)
        fclose(hFile);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
    int overwrite = 0;
    const char *InFile, *OutFile;
    size_t FileSize;
    utkheader_t UTKHeader;
    clock_t BeginningTime, EndingTime;

    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: utkdecode [-f] infile outfile\n"
        " -or-  utkdecode infile\n"
        "Transcode or play a Maxis UTalk sound.\n"
        "Use -f to force overwriting without confirmation.\n"
        "\n"
        "Report bugs to <X-Fi6@phppoll.org>.\n"
        "utkdecode is maintained by the Niotso project.\n"
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
    if(FileSize < 32)
        Shutdown_M("Not a valid UTK file");
    fseek(hFile, 0, SEEK_SET);

    UTKData = malloc(FileSize);
    if(UTKData == NULL)
        Shutdown_M("Memory for this file could not be allocated");
    if(fread(UTKData, 1, FileSize, hFile) != FileSize)
        Shutdown_M("The input file could not be read");
    fclose(hFile); hFile = NULL;

    /****
    ** Transcode the data from UTK to LPCM
    */

    if(!utk_read_header(&UTKHeader, UTKData, FileSize))
        Shutdown_M("Not a valid UTK file");

    if(argc >= 3){ /* Transcode */
        WaveData = malloc(44+UTKHeader.dwOutSize);
        if(WaveData == NULL)
            Shutdown_M("Memory for this file could not be allocated");

        BeginningTime = clock();
        if(!utk_decode(UTKData+32, WaveData+44, FileSize-32, UTKHeader.Frames))
            Shutdown_M("Memory for this file could not be allocated");
        EndingTime = clock();

        free(UTKData); UTKData = NULL;

        /****
        ** Write the Microsoft WAV header
        */

        write_uint32(WaveData, 0x46464952); /* "RIFF" */
        write_uint32(WaveData+4, 36+UTKHeader.dwOutSize);
        write_uint32(WaveData+8, 0x45564157); /* "WAVE" */
        write_uint32(WaveData+12, 0x20746d66); /* "fmt " */
        write_uint32(WaveData+16, 16);
        write_uint16(WaveData+20, 1);
        write_uint16(WaveData+22, UTKHeader.nChannels);
        write_uint32(WaveData+24, UTKHeader.nSamplesPerSec);
        write_uint32(WaveData+28, UTKHeader.nAvgBytesPerSec);
        write_uint16(WaveData+32, UTKHeader.nBlockAlign);
        write_uint16(WaveData+34, UTKHeader.wBitsPerSample);
        write_uint32(WaveData+36, 0x61746164); /* "data" */
        write_uint32(WaveData+40, UTKHeader.dwOutSize);

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
        printf("Extracted %u bytes in %.2f seconds.\n", (unsigned) UTKHeader.dwOutSize,
            ((float)(EndingTime - BeginningTime))/CLOCKS_PER_SEC);
        fwrite(WaveData, 1, 44+UTKHeader.dwOutSize, hFile);
        free(WaveData);
        fclose(hFile);
    }

    return 0;
}
