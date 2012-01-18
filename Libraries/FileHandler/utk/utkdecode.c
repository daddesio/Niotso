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

#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include "read_utk.h"

#ifndef write_int32
 #define write_uint32(dest, src) \
    (dest)[0] = ((src)&0x000000FF)>>(8*0); \
    (dest)[1] = ((src)&0x0000FF00)>>(8*1); \
    (dest)[2] = ((src)&0x00FF0000)>>(8*2); \
    (dest)[3] = ((src)&0xFF000000)>>(8*3)
 #define write_uint16(dest, src) \
    (dest)[0] = ((src)&0x00FF)>>(8*0); \
    (dest)[1] = ((src)&0xFF00)>>(8*1)
#endif

int main(int argc, char *argv[]){
    int overwrite = 0;
    char *InFile, *OutFile;
    HANDLE hFile;
    HANDLE ProcessHeap = GetProcessHeap();
    unsigned FileSize;
    DWORD bytestransferred = 0;
    uint8_t * UTKData;
    utkheader_t UTKHeader;
    unsigned BeginningTime, EndingTime;

    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: utkdecode [-f] infile outfile\n"
        " -or-  utkdecode infile\n"
        "Transcode or play a Maxis UTalk sound.\n"
        "Use -f to force overwriting without confirmation.\n"
        "\n"
        "Report bugs to <X-Fi6@phppoll.org>.\n"
        "utkdecode is maintained by the Niotso project.\n"
        "Home page: <http://www.niotso.org/>");
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

    hFile = CreateFile(InFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            printf("%sThe specified input file does not exist.", "utkdecode: error: ");
            return -1;
        }
        printf("%sThe input file could not be opened for reading.", "utkdecode: error: ");
        return -1;
    }
    FileSize = GetFileSize(hFile, NULL);
    if(FileSize < 24){
        printf("%sNot a valid UTK file.", "utkdecode: error: ");
        return -1;
    }
    UTKData = HeapAlloc(ProcessHeap, HEAP_NO_SERIALIZE, FileSize);
    if(UTKData == NULL){
        printf("%sMemory for this file could not be allocated.", "utkdecode: error: ");
        return -1;
    }
    if(!ReadFile(hFile, UTKData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        printf("%sThe input file could not be read.", "utkdecode: error: ");
        return -1;
    }
    CloseHandle(hFile);

    /****
    ** Transcode the data from UTK to LPCM
    */

    if(!utk_read_header(&UTKHeader, UTKData, FileSize)){
        printf("%sNot a valid UTK file.", "utkdecode: error: ");
        return -1;
    }

    if(argc >= 3){ /* Transcode */
        uint8_t * WaveData = HeapAlloc(ProcessHeap, HEAP_NO_SERIALIZE, 44+UTKHeader.dwOutSize);
        if(WaveData == NULL){
            printf("%sMemory for this file could not be allocated.", "utkdecode: error: ");
            return -1;
        }

        UTKGenerateTables();
        
        BeginningTime = GetTickCount();
        if(!utk_decode(UTKData+32, WaveData+44, UTKHeader.Frames)){
            printf("%sMemory for this file could not be allocated.", "utkdecode: error: ");
            return -1;
        }
        EndingTime = GetTickCount();

        HeapFree(ProcessHeap, HEAP_NO_SERIALIZE, UTKData);

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

        hFile = CreateFile(OutFile, GENERIC_WRITE, 0, NULL, CREATE_NEW+overwrite,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if(hFile == INVALID_HANDLE_VALUE){
            if(!overwrite && GetLastError() == ERROR_FILE_EXISTS){
                char c;
                printf("File \"%s\" exists.\nContinue anyway? (y/n) ", OutFile);
                c = getchar();
                if(c != 'y' && c != 'Y'){
                    printf("\nAborted.");
                    return -1;
                }
                hFile = CreateFile(OutFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            }
            if(hFile == INVALID_HANDLE_VALUE){
                printf("%sThe output file could not be opened for writing.", "utkdecode: error: ");
                return -1;
            }
        }
        printf("Extracted %u bytes in %.2f seconds.\n", (unsigned) UTKHeader.dwOutSize,
            ((float) (EndingTime - BeginningTime))/1000);
        WriteFile(hFile, WaveData, 44+UTKHeader.dwOutSize, &bytestransferred, NULL);
        CloseHandle(hFile);
    }

    return 0;
}