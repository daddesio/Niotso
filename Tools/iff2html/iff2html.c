/*
    iff2html - iff web page description generator
    iff2html.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
    Author(s): Fatbag <X-Fi6@phppoll.org>
               Ahmed El-Mahdawy <aa.mahdawy.10@gmail.com>

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
#include <stdio.h>
#include <stdint.h>
#include <iff/iff.h>
#include "md5.h"
#include "image.h"

#ifndef min
 #define min(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
 #define max(x,y) ((x) > (y) ? (x) : (y))
#endif

static void printsize(FILE * hFile, size_t FileSize){
    /* For our purposes, our units are best described in kB and MB, if not bytes */
    size_t temp = FileSize;
    unsigned position = 1;
    if(FileSize >= 1048576)
        fprintf(hFile, "%.1f MB (", (float)FileSize/1048576);
    else
        fprintf(hFile, "%.1f kB (", (float)FileSize/1024);
    while((temp/=1000) != 0)
        position *= 1000;
    fprintf(hFile, "%u", (unsigned) FileSize/position);
    FileSize -= (FileSize/position)*position;
    while((position/=1000) != 0){
        fprintf(hFile, ",%.3u", (unsigned) FileSize/position);
        FileSize -= (FileSize/position)*position;
    }
    fprintf(hFile, " bytes)");
}

int main(int argc, char *argv[]){
    unsigned c;
    int slash;
    FILE * hFile;
    int overwrite = 0;
    char *InFile, *OutFile = NULL, *FileName, *OutDir = NULL;
    size_t FileSize;
    struct MD5Context md5c;
    unsigned char digest[16];
    uint8_t * IFFData;
    IFFFile IFFFileInfo;
    IFFChunk * ChunkData;

    if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
        printf("Usage: iff2html [-f] infile (outfile)\n"
        "Produce an HTML webpage describing an EA IFF file.\n"
        "Use -f to force overwriting without confirmation.\n"
        "If outfile is unspecified, file.iff will output to file.html.\n"
        "\n"
        "Report bugs to <X-Fi6@phppoll.org>.\n"
        "iff2html is maintained by the Niotso project.\n"
        "Home page: <http://www.niotso.org/>\n");
        return 0;
    }

    if(argc >= 4 && !strcmp(argv[1], "-f")){
        overwrite++;
        InFile = argv[2];
        OutFile = argv[3];
    }else if(argc == 3){
        if(!strcmp(argv[1], "-f")){
            overwrite++;
            InFile = argv[2];
        }else{
            InFile = argv[1];
            OutFile = argv[2];
        }
    }else InFile = argv[1];
    if(OutFile == NULL){
        unsigned length = strlen(InFile);
        OutFile = malloc(max(length+2, 6));
        strcpy(OutFile, InFile);
        strcpy(max(OutFile+length-4, OutFile), ".html");
    }

    for(c=0, slash=-1; OutFile[c]; c++)
        if(OutFile[c] == '/' || OutFile[c] == '\\') slash = c;
    if(slash >= 0){
        OutDir = malloc(slash+2);
        memcpy(OutDir, OutFile, slash+1);
        OutDir[slash+1] = 0x00;
    }else OutDir = "";

    for(c=0, slash=-1; InFile[c]; c++)
        if(InFile[c] == '/' || InFile[c] == '\\') slash = c;
    FileName = InFile + slash + 1;

    /****
    ** Open the file and read in entire contents to memory
    */

    hFile = fopen(InFile, "rb");
    if(hFile == NULL){
        printf("%sThe specified input file does not exist or could not be opened for reading.", "iff2html: error: ");
        return -1;
    }
    fseek(hFile, 0, SEEK_END);
    FileSize = ftell(hFile);
    if(FileSize < 64){
        fclose(hFile);
        printf("%sNot a valid IFF file.", "iff2html: error: ");
        return -1;
    }
    fseek(hFile, 0, SEEK_SET);

    IFFData = malloc(FileSize);
    if(IFFData == NULL){
        fclose(hFile);
        printf("%sMemory for this file could not be allocated.", "iff2html: error: ");
        return -1;
    }
    if(fread(IFFData, 1, FileSize, hFile) != FileSize){
        fclose(hFile);
        printf("%sThe input file could not be read.", "iff2html: error: ");
        return -1;
    }
    fclose(hFile);

    /****
    ** Load header information
    */

    if(!iff_create(&IFFFileInfo)){
        printf("%sMemory for this file could not be allocated.", "iff2html: error: ");
        return -1;
    }
    if(!iff_read_header(&IFFFileInfo, IFFData, FileSize)){
        printf("%sNot a valid IFF file.", "iff2html: error: ");
        return -1;
    }

    /****
    ** Load entry information
    */

    if(!iff_enumerate_chunks(&IFFFileInfo, IFFData+64, FileSize-64)){
        printf("%sChunk data is corrupt.", "iff2html: error: ");
        return -1;
    }

    /* Calculate the MD5, and then we can free the IFF data because we're done with it */
    MD5Init(&md5c);
    MD5Update(&md5c, IFFData, FileSize);
    MD5Final(digest, &md5c);
    free(IFFData);

    for(c = 0, ChunkData = IFFFileInfo.Chunks; c < IFFFileInfo.ChunkCount; c++, ChunkData++)
        iff_parse_chunk(ChunkData, ChunkData->Data);

    /****
    ** Open the output file and write the header
    */
    if(!overwrite){
        hFile = fopen(OutFile, "rb");
        if(hFile != NULL){
            /* File exists */
            char c;
            fclose(hFile);
            printf("File \"%s\" exists.\nContinue anyway? (y/n) ", OutFile);
            c = getchar();
            if(c != 'y' && c != 'Y'){
                printf("\nAborted.");
                return -1;
            }
        }
    }
    hFile = fopen(OutFile, "wb");
    if(hFile == NULL){
        printf("%sThe output file could not be opened for writing.", "iff2html: error: ");
        return -1;
    }

    /****
    ** We're splitting fprintf by line to guarantee compatibility;
    ** even C99 compilers are only required to support 4096 byte strings in printf()-related functions
    */
    fprintf(hFile,
        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
    fprintf(hFile, "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\" dir=\"ltr\">\n");
    fprintf(hFile, "<head>\n");
    fprintf(hFile, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />\n");
    fprintf(hFile, "<meta http-equiv=\"Content-Style-Type\" content=\"text/css; charset=iso-8859-1\" />\n");
    fprintf(hFile, "<meta http-equiv=\"Content-Language\" content=\"en\" />\n");
    fprintf(hFile, "<meta name=\"description\" content=\"%s (iff2html)\" />\n", FileName);
    fprintf(hFile, "<meta name=\"generator\" content=\"iff2html\" />\n");
    fprintf(hFile, "<title>%s (iff2html)</title>\n", FileName);
    fprintf(hFile, "<style type=\"text/css\" media=\"all\">\n");
    fprintf(hFile, "html, body {\n");
    fprintf(hFile, "    background: #fff;\n");
    fprintf(hFile, "    color: #000;\n");
    fprintf(hFile, "    font-family: sans-serif;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "\n");
    fprintf(hFile, "a:link, a:visited, a:hover, a:active { color: #00f; }\n");
    fprintf(hFile, "a:link, a:visited { text-decoration: none; }\n");
    fprintf(hFile, "a:hover, a:active { text-decoration: underline; }\n");
    fprintf(hFile, "\n");
    fprintf(hFile, "#attributes {\n");
    fprintf(hFile, "    border-left: 2px solid #888; padding-left: 4px; margin-bottom: 1em;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "\n");
    fprintf(hFile, "#toc {\n");
    fprintf(hFile, "    display: table-cell;\n");
    fprintf(hFile, "    margin-top: 1em;\n");
    fprintf(hFile, "    background: #eee; border: 1px solid #bbb;\n");
    fprintf(hFile, "    padding: .25em;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "#toc div {\n");
    fprintf(hFile, "    border-bottom: 1px solid #aaa;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "#toc ul {\n");
    fprintf(hFile, "    list-style-type: none;\n");
    fprintf(hFile, "    padding: 0;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "ul ul {\n");
    fprintf(hFile, "    padding: 2em;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "\n");
    fprintf(hFile, "h2 {\n");
    fprintf(hFile, "    border-bottom: 1px solid #888;\n");
    fprintf(hFile, "    margin: 2em 0 0.25em 0;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "h2 a {\n");
    fprintf(hFile, "    font-size: 9pt;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "\n");
    fprintf(hFile, "table {\n");
    fprintf(hFile, "    border: 1px #aaa solid;\n");
    fprintf(hFile, "    border-collapse: collapse;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "th, td {\n");
    fprintf(hFile, "    border: 1px #aaa solid;\n");
    fprintf(hFile, "    padding: 0.2em;\n");
    fprintf(hFile, "    white-space: pre-wrap;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "\n");
    fprintf(hFile, ".center {\n");
    fprintf(hFile, "    margin: auto auto;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, ".centerall * {\n");
    fprintf(hFile, "    text-align: center;\n");
    fprintf(hFile, "    vertical-align: middle;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "\n");
    fprintf(hFile, ".palette td, .palette th {\n");
    fprintf(hFile, "    border: none;\n");
    fprintf(hFile, "    width: 16px;\n");
    fprintf(hFile, "    height: 16px;\n");
    fprintf(hFile, "    font-size: 12px;\n");
    fprintf(hFile, "    line-height: 16px;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, ".palette td[title] {\n");
    fprintf(hFile, "    border: 1px solid #000;\n");
    fprintf(hFile, "    cursor: help;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "\n");
    fprintf(hFile, "#footer {\n");
    fprintf(hFile, "    margin-top: 2em;\n");
    fprintf(hFile, "    padding-bottom: 0.5em;\n");
    fprintf(hFile, "    text-align: center;\n");
    fprintf(hFile, "}\n");
    fprintf(hFile, "</style>\n");
    fprintf(hFile, "</head>\n");
    fprintf(hFile, "<body>\n");
    fprintf(hFile, "<h1>%s</h1>\n", FileName);
    fprintf(hFile, "<div id=\"attributes\">\n");
    fprintf(hFile, "<div>");
    for(c=0; c<16; c++)
        fprintf(hFile, "%.2x", digest[c]);
    fprintf(hFile, " (md5), ");
    printsize(hFile, FileSize);
    fprintf(hFile, "</div>\n");
    fprintf(hFile, "<div>Dumped by iff2html.</div></div>\n");
    fprintf(hFile, "\n");
    fprintf(hFile, "<div id=\"toc\"><div><b>Contents</b> &ndash; %u chunks</div>\n", IFFFileInfo.ChunkCount);
    fprintf(hFile, "<ul>\n");
    for(c=1, ChunkData = IFFFileInfo.Chunks; c <= IFFFileInfo.ChunkCount; c++, ChunkData++)
        fprintf(hFile, "<li><a href=\"#chunk%u_%.4x\">%u [%s] (%.4X)%s%s</a></li>\n",
            c, ChunkData->ChunkID, c, ChunkData->Type, ChunkData->ChunkID,
            (ChunkData->Label[0] != 0x00) ? " &ndash; " : "", ChunkData->Label);
    fprintf(hFile, "</ul>\n");
    fprintf(hFile, "</div>\n");
    fprintf(hFile, "\n");

    for(c=0, ChunkData = IFFFileInfo.Chunks; c < IFFFileInfo.ChunkCount; c++, ChunkData++){
        fprintf(hFile, "<h2 id=\"chunk%u_%.4x\">%u [%s] (%.4X)%s%s <a href=\"#chunk%u_%.4x\">(Jump)</a></h2>\n",
            c+1, ChunkData->ChunkID, c+1, ChunkData->Type, ChunkData->ChunkID,
            (ChunkData->Label[0] != 0x00) ? " &ndash; " : "", ChunkData->Label,
            c+1, ChunkData->ChunkID);
        fprintf(hFile, "<div>\n");

        if(ChunkData->FormattedData == NULL){
            int success = 0;
            /* The iff library does not parse BMP_ or FBMP chunks */
            if(!strcmp(ChunkData->Type, "BMP_") || !strcmp(ChunkData->Type, "FBMP")){
                int bmp = !strcmp(ChunkData->Type, "BMP_");
                size_t Width, Height;
                char filename[32];
                sprintf(filename, "%s%s_%u_%.4x.png", OutDir, bmp ? "bmp" : "fbmp", c+1, ChunkData->ChunkID);

                if(WritePNG(filename, ChunkData, 0, NULL, &Width, &Height)){
                    fprintf(hFile, "<table class=\"center centerall\">\n");
                    fprintf(hFile, "<tr><th>Image</th></tr>\n");
                    fprintf(hFile, "<tr><td><img src=\"%s_%u_%.4x.png\" width=\"%u\" height=\"%u\" alt=\"\" /></td></tr>\n",
                        bmp ? "bmp" : "fbmp", c+1, ChunkData->ChunkID, (unsigned) Width, (unsigned) Height);
                    fprintf(hFile, "</table>\n");
                    success++;
                }
            }
            if(!success)
                fprintf(hFile, "The contents of this chunk could not be parsed.\n");
        }else if(!strcmp(ChunkData->Type, "STR#")  ||
            !strcmp(ChunkData->Type, "CTSS") ||
            !strcmp(ChunkData->Type, "FAMs") ||
            !strcmp(ChunkData->Type, "TTAs") ||
            !strcmp(ChunkData->Type, "CST")  ){
            /****
            ** STR# parsing
            */

            IFFString * StringData = ChunkData->FormattedData;
            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Format:</td><td>");
            switch(StringData->Format){
                case 0:  fprintf(hFile, "<tt>00 00</tt> (0)"); break;
                case -1: fprintf(hFile, "<tt>FF FF</tt> (&minus;1)"); break;
                case -2: fprintf(hFile, "<tt>FE FF</tt> (&minus;2)"); break;
                case -3: fprintf(hFile, "<tt>FD FF</tt> (&minus;3)"); break;
                case -4: fprintf(hFile, "<tt>FC FF</tt> (&minus;4)"); break;
                default: fprintf(hFile, "Unrecognized"); break;
            }
            fprintf(hFile, "</td></tr>\n");
            fprintf(hFile, "</table>\n");
            if(StringData->Format >= -4 && StringData->Format <= 0){
                unsigned LanguageSet;
                const char * LanguageStrings[] = {
                    "English (US)",
                    "English (International)",
                    "French",
                    "German",
                    "Italian",
                    "Spanish",
                    "Dutch",
                    "Danish",
                    "Swedish",
                    "Norwegian",
                    "Finnish",
                    "Hebrew",
                    "Russian",
                    "Portuguese",
                    "Japanese",
                    "Polish",
                    "Simplified Chinese",
                    "Traditional Chinese",
                    "Thai",
                    "Korean"
                };
                fprintf(hFile, "<br />\n");
                fprintf(hFile, "<table class=\"center\">\n");
                fprintf(hFile, "<tr><th>Language</th><th colspan=\"3\">String pairs</th></tr>\n");

                for(LanguageSet=0; LanguageSet<20; LanguageSet++){
                    IFFStringPair * Pair;
                    unsigned PairIndex;
                    if(StringData->LanguageSets[LanguageSet].PairCount == 0)
                        continue;

                    fprintf(hFile, "<tr><td rowspan=\"%u\">%s</td>\n", StringData->LanguageSets[LanguageSet].PairCount,
                        LanguageStrings[LanguageSet]);
                    for(PairIndex=1, Pair = StringData->LanguageSets[LanguageSet].Pairs;
                        PairIndex <= StringData->LanguageSets[LanguageSet].PairCount; PairIndex++, Pair++){
                        if(PairIndex != 1)
                            fprintf(hFile, "<tr>");
                        fprintf(hFile, "<td>%u</td><td>%s</td><td>%s</td></tr>\n", PairIndex,
                            (Pair->Key)   != NULL ? Pair->Key   : "",
                            (Pair->Value) != NULL ? Pair->Value : "");
                    }
                }

                fprintf(hFile, "</table>\n");
            }
        }else if(!strcmp(ChunkData->Type, "CATS")){
            /****
            ** Regular string pair
            */

            IFFStringPair * Pair = ChunkData->FormattedData;

            fprintf(hFile, "<table class=\"center\">\n");
            fprintf(hFile, "<tr><th>Key</th><th>Value</th></tr>\n");
            fprintf(hFile, "<tr><td>%s</td><td>%s</td></tr>\n",
                (Pair->Key)   != NULL ? Pair->Key   : "",
                (Pair->Value) != NULL ? Pair->Value : "");
            fprintf(hFile, "</table>\n");
        }else if(!strcmp(ChunkData->Type, "FWAV") || !strcmp(ChunkData->Type, "GLOB")){
            /****
            ** Regular string
            */

            fprintf(hFile, "<table class=\"center\">\n");
            fprintf(hFile, "<tr><th>String</th></tr>\n");
            fprintf(hFile, "<tr><td>%s</td></tr>\n", ChunkData->FormattedData ? (char*) ChunkData->FormattedData : "");
            fprintf(hFile, "</table>\n");
        }else if(!strcmp(ChunkData->Type, "BCON")){
            /****
            ** BCON parsing
            */

            IFF_BCON * BCONData = ChunkData->FormattedData;
            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Flags:</td><td><tt>%02X</tt> (%u)</td></tr>\n", BCONData->Flags, BCONData->Flags);
            fprintf(hFile, "</table>\n");
            if(BCONData->ConstantCount > 0){
                unsigned i;

                fprintf(hFile, "<br />\n");
                fprintf(hFile, "<table class=\"center\">\n");
                fprintf(hFile, "<tr><th colspan=\"2\">Constant Value</th></tr>\n");
                for(i=0; i<BCONData->ConstantCount; i++)
                    fprintf(hFile, "<tr><td>%u</td><td>%u</td></tr>\n", i+1, BCONData->Constants[i]);
                fprintf(hFile, "</table>\n");
            }
        }else if(!strcmp(ChunkData->Type, "FCNS")){
            /****
            ** FCNS parsing
            */

            IFFConstantList * List = ChunkData->FormattedData;
            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Version:</td><td>%u</td></tr>\n", List->Version);
            fprintf(hFile, "</table>\n");
            if(List->ConstantCount > 0){
                IFFConstant * Constant;
                unsigned i;

                fprintf(hFile, "<br />\n");
                fprintf(hFile, "<table class=\"center\">\n");
                fprintf(hFile, "<tr><th colspan=\"2\">Name</th><th>Value</th><th>Description</th></tr>\n");
                for(i=0, Constant=List->Constants; i<List->ConstantCount; i++, Constant++)
                    fprintf(hFile, "<tr><td>%u</td><td>%s</td><td>%g</td><td>%s</td></tr>\n",
                        i+1,
                        Constant->Name ? Constant->Name : "",
                        Constant->Value,
                        Constant->Description ? Constant->Description : "");
                fprintf(hFile, "</table>\n");
            }
        }else if(!strcmp(ChunkData->Type, "TMPL")){
            /****
            ** TMPL parsing
            */

            IFFTemplate * Template = ChunkData->FormattedData;
            IFFTemplateField * Field;
            unsigned i;
            fprintf(hFile, "<table class=\"center\">\n");
            fprintf(hFile, "<tr><th colspan=\"2\">Name</th><th>Type</th>\n");
            for(i=0, Field=Template->Fields; i<Template->FieldCount; i++, Field++)
                fprintf(hFile, "<tr><td>%u</td><td>%s</td><td>%s</td></tr>\n",
                    i+1,
                    Field->Name ? Field->Name : "",
                    Field->Type ? Field->Type : "");
            fprintf(hFile, "</table>\n");
        }else if(!strcmp(ChunkData->Type, "TRCN")){
            /****
            ** TRCN parsing
            */

            IFFRangeSet * RangeSet = ChunkData->FormattedData;
            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Version:</td><td>%u</td></tr>\n", RangeSet->Version);
            fprintf(hFile, "</table>\n");
            if(RangeSet->RangeCount > 0){
                unsigned i;
                IFFRangeEntry * Range;

                fprintf(hFile, "<br />\n");
                fprintf(hFile, "<table class=\"center\">\n");
                fprintf(hFile, "<tr><th colspan=\"2\">In use</th><th>Default value</th><th>Name</th>"
                    "<th>Comment</th><th>Range is enforced</th><th>Minimum</th><th>Maximum</th></tr>\n");
                for(i=0, Range=RangeSet->Ranges; i<RangeSet->RangeCount; i++, Range++)
                    fprintf(hFile,
                        "<tr><td>%u</td><td>%s</td><td>%u</td><td>%s</td><td>%s</td><td>%s</td><td>%u</td><td>%u</td></tr>\n",
                        i+1,
                        Range->IsUsed ? "Yes" : "No", Range->DefaultValue,
                        Range->Name ? Range->Name : "",
                        Range->Comment ? Range->Comment : "",
                        Range->Enforced ? "Yes" : "No",
                        Range->RangeMin, Range->RangeMax);
                fprintf(hFile, "</table>\n");
            }
        }else if(!strcmp(ChunkData->Type, "PALT")){
            /****
            ** PALT parsing
            */

            IFFPalette * Palette = ChunkData->FormattedData;
            uint8_t * Data = Palette->Data;
            unsigned i, j;

            fprintf(hFile, "<table class=\"center palette\" border=\"0\">\n");
            fprintf(hFile, "<tr><th></th>");
            for(i=0; i<16; i++) fprintf(hFile, "<th>%X</th>", i);
            fprintf(hFile, "</tr>\n");
            for(i=0; i<16; i++){
                fprintf(hFile, "<tr><th>%X</th>", i);
                for(j=0; j<16; j++){
                    if(i*16 + j < Palette->ColorCount){
                        unsigned red = *(Data++);
                        unsigned green = *(Data++);
                        unsigned blue = *(Data++);

                        fprintf(hFile, "\n<td style=\"background:#%.2x%.2x%.2x\" title=\"%u: #%.2x%.2x%.2x\"></td>",
                            red, green, blue, i*16 + j, red, green, blue);
                    }else
                        fprintf(hFile, "\n<td></td>");
                }
                fprintf(hFile, "</tr>\n");
            }
            fprintf(hFile, "</table>\n");
        }else if(!strcmp(ChunkData->Type, "SPR#") || !strcmp(ChunkData->Type, "SPR2")){
            /****
            ** SPR# and SPR2 parsing
            */

            int spr1 = !strcmp(ChunkData->Type, "SPR#");
            IFFSpriteList * SpriteList = ChunkData->FormattedData;
            IFFChunk * Palette = NULL;
            IFFPalette BlankPalette;
            IFFPalette * PaletteData;
            unsigned i;

            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Version:</td><td>%u</td></tr>\n", SpriteList->Version);
            fprintf(hFile, "<tr><td>Palette ID:</td><td>%.4X</td></tr>\n", SpriteList->PaletteID);
            fprintf(hFile, "</table>\n");

            if(SpriteList->PaletteID < 0xFFFF){
                Palette = iff_find_chunk(&IFFFileInfo, "PALT", SpriteList->PaletteID);
                if(!Palette || !Palette->FormattedData) Palette = iff_find_chunk(&IFFFileInfo, "PALT", ChunkData->ChunkID);
                if(!Palette || !Palette->FormattedData) Palette = iff_find_chunk(&IFFFileInfo, "PALT", -1);
            }
            if(!Palette || !Palette->FormattedData){
                memset(&BlankPalette, 0, sizeof(IFFPalette));
                BlankPalette.Version = 1;
                BlankPalette.ColorCount = 256;
                PaletteData = &BlankPalette;
            }else PaletteData = Palette->FormattedData;

            fprintf(hFile, "<table class=\"center centerall\">\n");
            fprintf(hFile, "<tr><th colspan=\"2\">Sprite</th>");
            if(!spr1) fprintf(hFile, "<th>Z-Buffer</th>");
            fprintf(hFile, "</tr>\n");
            for(i=0; i<SpriteList->SpriteCount; i++){
                IFFSprite * Sprite = &SpriteList->Sprites[i];
                char filename[32];
                sprintf(filename, "%s%s_%u_%.4x_%u.png", OutDir, spr1 ? "spr1" : "spr2", c+1, ChunkData->ChunkID, i+1);

                fprintf(hFile, "<tr><td>%u</td><td", i+1);
                if(Sprite->IndexData && iff_depalette(Sprite, PaletteData)){
                    WritePNG(filename, NULL, 0, Sprite, NULL, NULL);
                    fprintf(hFile, "><img src=\"%s_%u_%.4x_%u.png\" width=\"%u\" height=\"%u\" alt=\"\" />",
                        spr1 ? "spr1" : "spr2", c+1, ChunkData->ChunkID, i+1, Sprite->Width, Sprite->Height);
                    if(!spr1){
                        sprintf(filename, "%sspr2_%u_%.4x_%u_z.png", OutDir, c+1, ChunkData->ChunkID, i+1);
                        if(Sprite->ZBuffer){
                            WritePNG(filename, NULL, 1, Sprite, NULL, NULL);
                            fprintf(hFile, "</td><td><img src=\"spr2_%u_%.4x_%u_z.png\" width=\"%u\" height=\"%u\" alt=\"\" />",
                                c+1, ChunkData->ChunkID, i+1, Sprite->Width, Sprite->Height);
                        }else
                            fprintf(hFile, "None provided");
                    }
                }else
                    fprintf(hFile, Sprite->InvalidDimensions ? "%sBlank sprite" : "%sThis sprite cannot be displayed.",
                        !spr1 ? " colspan=\"2\">" : ">");
                fprintf(hFile, "</td></tr>\n");
            }
            fprintf(hFile, "</table>\n");
        }else if(!strcmp(ChunkData->Type, "DGRP")){
            /****
            ** DGRP parsing
            */

            IFFDrawGroup * Group = ChunkData->FormattedData;
            IFFDrawAngle * Angle;
            IFFSpriteInfo * Sprite;
            unsigned i,j;
            const char * Zooms[] = {"Far", "Middle", "Close"};

            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Version:</td><td>%u</td></tr>\n", Group->Version);
            fprintf(hFile, "</table>\n");
            fprintf(hFile, "<br />\n");

            fprintf(hFile, "<table class=\"center\">\n");
            fprintf(hFile, "<tr><th>Direction</th><th>Zoom</th><th colspan=\"6\">Sprite</th></tr>\n");
            for(i=0, Angle=Group->DrawAngles; i<12; i++, Angle++){
                const char * Direction =
                    (Angle->Direction == IFFDIRECTION_NORTHEAST)  ? "North east" :
                    (Angle->Direction == IFFDIRECTION_SOUTHEAST)  ? "South east" :
                    (Angle->Direction == IFFDIRECTION_NORTHWEST)  ? "North west" :
                                                                    "South west";

                if(Angle->SpriteCount){
                    fprintf(hFile,
                        "<tr><td rowspan=\"%u\">%s</td><td rowspan=\"%u\">%s</td>"
                        "<th>#</th><th>Type</th><th>Chunk ID</th><th>Sprite index</th>"
                        "<th>Flags</th><th>Sprite offset</th><th>Object offset</th></tr>\n",
                        1+Angle->SpriteCount, Direction, 1+Angle->SpriteCount, Zooms[Angle->Zoom-1]);
                    for(j=0, Sprite = Angle->SpriteInfo; j<Angle->SpriteCount; j++, Sprite++)
                        fprintf(hFile, "<tr><td>%u</td><td>%u</td><td>%.4X</td><td>%u</td><td>%u</td>"
                            "<td>(%+d,%+d)</td><td>(%+g,%+g,%+g)</td></tr>",
                            j+1, Sprite->Type, Sprite->ChunkID, Sprite->SpriteIndex, Sprite->Flags,
                            Sprite->SpriteX, Sprite->SpriteY, Sprite->ObjectX, Sprite->ObjectY, Sprite->ObjectZ);

                }else{
                    fprintf(hFile, "<tr><td>%s</td><td>%s</td><td>None specified</td></tr>", Direction, Zooms[Angle->Zoom-1]);
                }
            }
            fprintf(hFile, "</table>\n");
        }else if(!strcmp(ChunkData->Type, "BHAV")){
            /****
            ** BHAV parsing
            */

            IFFBehavior * Behavior = ChunkData->FormattedData;
            IFFInstruction * Instruction;

            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Version:</td><td>%u</td></tr>\n", Behavior->Version);
            fprintf(hFile, "<tr><td>Type:</td><td>%u</td></tr>\n", Behavior->Type);
            fprintf(hFile, "<tr><td>Arguments:</td><td>%u</td></tr>\n", Behavior->ArgumentCount);
            fprintf(hFile, "<tr><td>Locals:</td><td>%u</td></tr>\n", Behavior->LocalCount);
            fprintf(hFile, "<tr><td>Flags:</td><td>%.4X</td></tr>\n", Behavior->Flags);
            fprintf(hFile, "</table>\n");

            if(Behavior->InstructionCount > 0){
                unsigned i;

                fprintf(hFile, "<br />\n");
                fprintf(hFile, "<table class=\"center\">\n");
                fprintf(hFile, "<tr><th colspan=\"2\">Opcode</th><th>T-Dest</th><th>F-Dest</th><th>Operand data</th></tr>\n");
                for(i=0, Instruction = Behavior->Instructions; i<Behavior->InstructionCount; i++, Instruction++)
                    fprintf(hFile, "<tr><td>%u</td><td><tt>%.4X</tt></td><td>%u</td><td>%u</td>"
                        "<td><tt>%.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X</tt></td></tr>\n",
                        i, Instruction->Opcode, Instruction->TDest, Instruction->FDest,
                        Instruction->Operands[0], Instruction->Operands[1],
                        Instruction->Operands[2], Instruction->Operands[3],
                        Instruction->Operands[4], Instruction->Operands[5],
                        Instruction->Operands[6], Instruction->Operands[7]);
                fprintf(hFile, "</table>\n");
            }
        }else if(!strcmp(ChunkData->Type, "OBJf")){
            /****
            ** OBJf parsing
            */

            IFFFunctionTable * Table = ChunkData->FormattedData;
            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Version:</td><td>%u</td></tr>\n", Table->Version);
            fprintf(hFile, "</table>\n");

            if(Table->FunctionCount > 0){
                unsigned i;

                fprintf(hFile, "<br />\n");
                fprintf(hFile, "<table class=\"center\">\n");
                fprintf(hFile, "<tr><th colspan=\"2\">Condition function</th><th>Action function</th></tr>\n");
                for(i=0; i<Table->FunctionCount; i++)
                    fprintf(hFile,
                        "<tr><td>%u</td><td>%.4X</td><td>%.4X</td></tr>\n",
                        i+1, Table->Functions[i].ConditionID, Table->Functions[i].ActionID);
                fprintf(hFile, "</table>\n");
            }
        }else{
            fprintf(hFile, "The contents of this chunk cannot be shown on this page.\n");
        }

        fprintf(hFile, "</div>\n\n");
    }
    iff_delete(&IFFFileInfo);

    fprintf(hFile,
        "<div id=\"footer\">This page was generated by the use of <a href=\"http://www.niotso.org/\">iff2html</a>.\n");
    fprintf(hFile, "The content of this page may be subject to copyright by the author(s) of the original iff file.</div>\n");
    fprintf(hFile, "</body>\n");
    fprintf(hFile, "</html>");
    fclose(hFile);

    printf("Wrote contents to '%s'.\n", OutFile);
    return 0;
}
