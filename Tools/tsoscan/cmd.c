/*
    tsoscan - IFF statistical webpage generator
    cmd.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Ahmed El-Mahdawy <aa.mahdawy.10@gmail.com>

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
#include "tsoscan.h"

CommandLineArgs* cmd_parse_args(int argc, char *argv[]){
    CommandLineArgs *CmdArgs = calloc(1, sizeof(CommandLineArgs));
    int i, InDirIndex = 0;

    if(CmdArgs == NULL) return 0;

    for(i=1; i<argc; i++){
        if(!strcmp(argv[i], "-o"))
            i++;
        else if(strcmp(argv[i], "-f") != 0)
            CmdArgs->InDirCount++;
    }
    if(CmdArgs->InDirCount > 0){
        CmdArgs->InDirs = calloc(CmdArgs->InDirCount, sizeof(char*));
        if(CmdArgs->InDirs == NULL) return 0;
    }

    for(i=1; i<argc; i++){
        if(!strcmp(argv[i], "-f")){
            CmdArgs->ForceWrite = 1;
        }else if(!strcmp(argv[i], "-o")){
            if(i == argc-1 || CmdArgs->OutFile != NULL)
                return NULL;
            CmdArgs->OutFile = argv[++i];
        }else{
            CmdArgs->InDirs[InDirIndex] = argv[i];
            InDirIndex++;
        }
    }

    return CmdArgs;
}

void cmd_delete(CommandLineArgs *args){
    unsigned i;

    if(args == NULL) return;
    if(args->InDirs != NULL){
        for(i=0; i<args->InDirCount; i++)
            free(args->InDirs[i]);
        free(args->InDirs);
    }
    free(args->OutFile);
    free(args);
}