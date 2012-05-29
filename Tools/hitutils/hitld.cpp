/*
    hitutils - The Sims HIT (dis)assembler and linker
    hitld.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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
#include <stdio.h>

int main(int argc, char *argv[]){
    unsigned objectcount;
    int arg;

    /****
    ** Parameter extraction
    */

    if(argc < 3 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: hitld [-f] [-hsm infile.hsm] [-hot infile.hot]\n"
        "       outfile.hit INFILES\n"
        "Link object files produced by hitasm into a HIT binary, and\n"
        "relink the game's HSM and HOT files.\n"
        "Use -f to force overwriting without confirmation.\n"
        "\n"
        "Report bugs to <X-Fi6@phppoll.org>.\n"
        "hitutils is maintained by the Niotso project.\n"
        "Home page: <http://www.niotso.org/>\n");
        return 0;
    }
    
    const char * hitfile;
    char * hsmfile = NULL, * hotfile = NULL;
    bool force = false;

    for(arg=1; arg<argc-2; arg++){
        if(!strcmp(argv[arg], "-f")) force = true;
        else if(arg<argc-3 && !strcmp(argv[arg], "-hsm")) hsmfile = argv[++arg];
        else if(arg<argc-3 && !strcmp(argv[arg], "-hot")) hotfile = argv[++arg];
        else break;
    }
    
    hitfile = argv[arg++];
    objectcount = argc-arg; //Guaranteed to be >=1
    
    for(int i=0, length = strlen(hitfile); i<2; i++){
        char *& string = (i==0) ? hsmfile : hotfile;
        if(!string){
            string = (char*) malloc(length+1);
            strcpy(string, hitfile);
            for(int j=1; j<=3 && j<=length; j++){
                const char * ext = "hsmhot";
                string[length-j] = ext[3*i + 3-j];
            }
        }
    }
    
    printf("Force: %s\nHSM file: %s\nHOT file: %s\nHIT file: %s\nObject count: %u",
        force ? "yes" : "no", hsmfile, hotfile, hitfile, objectcount);
    return 0;
}