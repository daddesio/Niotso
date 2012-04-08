/*
    iff2html.c - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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

#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <iff/iff.h>
#include "md5.h"

#ifndef min
 #define min(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
 #define max(x,y) ((x) > (y) ? (x) : (y))
#endif

void printsize(FILE * hFile, size_t FileSize){
    /* For our purposes, our units are best described in kB and MB */
    size_t temp = FileSize;
    unsigned position = 1;
    if(FileSize >= 1048576)
        fprintf(hFile, "%.1f MB (", (float)FileSize/1048576);
    else
        fprintf(hFile, "%.1f kB (", (float)FileSize/1024);
    while((temp/=1000) != 0)
        position *= 1000;
    fprintf(hFile, "%u", FileSize/position);
    FileSize -= (FileSize/position)*position;
    while((position/=1000) != 0){
        fprintf(hFile, ",%.3u", FileSize/position);
        FileSize -= (FileSize/position)*position;
    }
    fprintf(hFile, " bytes)");
}

int main(int argc, char *argv[]){
    unsigned i;
    FILE * hFile;
    int overwrite = 0;
    char *InFile, *OutFile = NULL;
    size_t FileSize;
    struct MD5Context md5c;
    unsigned char digest[16];
    uint8_t * IFFData;
    unsigned chunk = 0;
    IFFFile * IFFFileInfo;
    IFFChunkNode * ChunkNode;

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

    /****
    ** Open the file and read in entire contents to memory
    */

    hFile = CreateFile(InFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    hFile = fopen(InFile, "rb");
    if(hFile == NULL){
        printf("%sThe specified input file does not exist or could not be opened for reading.", "iff2html: error: ");
        return -1;
    }
    fseek(hFile, 0, SEEK_END);
    FileSize = ftell(hFile);
    if(FileSize < 64){
        printf("%sNot a valid IFF file.", "iff2html: error: ");
        return -1;
    }
    fseek(hFile, 0, SEEK_SET);

    IFFData = malloc(FileSize);
    if(IFFData == NULL){
        printf("%sMemory for this file could not be allocated.", "iff2html: error: ");
        return -1;
    }
    if(!fread(IFFData, FileSize, 1, hFile)){
        printf("%sThe input file could not be read.", "iff2html: error: ");
        return -1;
    }
    fclose(hFile);

    /****
    ** Load header information
    */

    IFFFileInfo = iff_create();
    if(IFFFileInfo == NULL){
        printf("%sMemory for this file could not be allocated.", "iff2html: error: ");
        return -1;
    }
    if(!iff_read_header(IFFFileInfo, IFFData, FileSize)){
        printf("%sNot a valid IFF file.", "iff2html: error: ");
        return -1;
    }

    /****
    ** Load entry information
    */

    if(!iff_enumerate_chunks(IFFFileInfo, IFFData+64, FileSize-64)){
        printf("%sChunk data is corrupt.", "iff2html: error: ");
        return -1;
    }
    
    /* Calculate the MD5, and then we can free the IFF data because we're done with it */
    MD5Init(&md5c);
    MD5Update(&md5c, IFFData, FileSize);
    MD5Final(digest, &md5c);
    free(IFFData);

    for(chunk = 1, ChunkNode = IFFFileInfo->FirstChunk; ChunkNode; ChunkNode = ChunkNode->NextChunk, chunk++)
        iff_parse_chunk(&ChunkNode->Chunk, ChunkNode->Chunk.Data);
    

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
    fprintf(hFile, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n");
    fprintf(hFile, "<meta http-equiv=\"Content-Style-Type\" content=\"text/css; charset=utf-8\" />\n");
    fprintf(hFile, "<meta http-equiv=\"Content-Language\" content=\"en\" />\n");
    fprintf(hFile, "<meta name=\"description\" content=\"%s (iff2html)\" />\n", InFile);
    fprintf(hFile, "<meta name=\"generator\" content=\"iff2html\" />\n");
    fprintf(hFile, "<title>%s (iff2html)</title>\n", InFile);
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
    fprintf(hFile, "}\n");
    fprintf(hFile, "\n");
    fprintf(hFile, ".center {\n");
    fprintf(hFile, "    margin: auto auto;\n");
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
    fprintf(hFile, "<h1>%s</h1>\n", InFile);
    fprintf(hFile, "<div id=\"attributes\">\n");
    fprintf(hFile, "<div>");
    for(i=0; i<16; i++)
        fprintf(hFile, "%.2x", digest[i]);
    fprintf(hFile, " (md5), ");
    printsize(hFile, FileSize);
    fprintf(hFile, "</div>\n");
    fprintf(hFile, "<div>Dumped by iff2html.</div></div>\n");
    fprintf(hFile, "\n");
    fprintf(hFile, "<div id=\"toc\"><div><b>Contents</b> &ndash; %u chunks</div>\n", IFFFileInfo->ChunkCount);
    fprintf(hFile, "<ul>\n");
    for(i=1, ChunkNode = IFFFileInfo->FirstChunk; ChunkNode; ChunkNode = ChunkNode->NextChunk, i++)
        fprintf(hFile, "<li><a href=\"#chunk%u_%.4x\">%u [%s] (%.4X)%s%s</a></li>\n",
            i, ChunkNode->Chunk.ChunkID, i, ChunkNode->Chunk.Type, ChunkNode->Chunk.ChunkID,
            (ChunkNode->Chunk.Label[0] != 0x00) ? " &ndash; " : "", ChunkNode->Chunk.Label);
    fprintf(hFile, "</ul>\n");
    fprintf(hFile, "</div>\n");
    fprintf(hFile, "\n");

    for(i=1, ChunkNode = IFFFileInfo->FirstChunk; ChunkNode; ChunkNode = ChunkNode->NextChunk, i++){
        IFF_STR * StringData = (IFF_STR*) ChunkNode->Chunk.FormattedData;
        fprintf(hFile, "<h2 id=\"chunk%u_%.4x\">%u [%s] (%.4X)%s%s <a href=\"#chunk%u_%.4x\">(Jump)</a></h2>\n",
            i, ChunkNode->Chunk.ChunkID, i, ChunkNode->Chunk.Type, ChunkNode->Chunk.ChunkID,
            (ChunkNode->Chunk.Label[0] != 0x00) ? " &ndash; " : "", ChunkNode->Chunk.Label,
            i, ChunkNode->Chunk.ChunkID);
        fprintf(hFile, "<div>\n");

        if(ChunkNode->Chunk.FormattedData == NULL){
            fprintf(hFile, "The contents of this chunk could not be parsed.\n");
        }else if(!strcmp(ChunkNode->Chunk.Type, "STR#")  ||
            !strcmp(ChunkNode->Chunk.Type, "CTSS") ||
            !strcmp(ChunkNode->Chunk.Type, "FAMs") ||
            !strcmp(ChunkNode->Chunk.Type, "TTAs") ){
            /****
            ** STR# parsing
            */

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
                    IFFStringPairNode * PairNode;
                    unsigned PairIndex;
                    if(StringData->LanguageSets[LanguageSet].PairCount == 0)
                        continue;

                    fprintf(hFile, "<tr><td rowspan=\"%u\">%s</td>\n", StringData->LanguageSets[LanguageSet].PairCount,
                        LanguageStrings[LanguageSet]);
                    for(PairIndex=1, PairNode = StringData->LanguageSets[LanguageSet].FirstPair; PairNode;
                        PairNode = PairNode->NextPair, PairIndex++){
                        if(PairIndex != 1)
                            fprintf(hFile, "<tr>");
                        fprintf(hFile, "<td>%u</td><td>%s</td><td>%s</td></tr>\n", PairIndex,
                            (PairNode->Pair.Key)   != NULL ? PairNode->Pair.Key   : "",
                            (PairNode->Pair.Value) != NULL ? PairNode->Pair.Value : "");
                    }
                }

                fprintf(hFile, "</table>\n");
            }
        }else if(!strcmp(ChunkNode->Chunk.Type, "BCON")){
            /****
            ** BCON parsing
            */
            
            IFF_BCON * BCONData = (IFF_BCON*) ChunkNode->Chunk.FormattedData;
            fprintf(hFile, "<table>\n");
            fprintf(hFile, "<tr><td>Flags:</td><td><tt>%02X</tt> (%d)</td></tr>\n", BCONData->Flags, BCONData->Flags);
            fprintf(hFile, "<tr><td>Number of Constants:</td><td>%u</td></tr>\n", BCONData->ConstantCount);
            fprintf(hFile, "</table>\n");
            if(BCONData->ConstantCount > 0){
                unsigned ConstantIndex;

                fprintf(hFile, "<br />\n");
                fprintf(hFile, "<table class=\"center\">\n");
                fprintf(hFile, "<tr><th colspan=\"2\">Constant Value</th></tr>\n");
                for(ConstantIndex=0; ConstantIndex<BCONData->ConstantCount; ConstantIndex++)
                    fprintf(hFile, "<tr><td>%u</td><td>%u</td></tr>\n", ConstantIndex+1, BCONData->Constants[ConstantIndex]);
                fprintf(hFile, "</table>\n");
            }
        }

        fprintf(hFile, "</div>\n\n");
    }

    fprintf(hFile,
        "<div id=\"footer\">This page was generated by the use of <a href=\"http://www.niotso.org/\">iff2html</a>.\n");
    fprintf(hFile, "The content of this page may be subject to copyright by the author(s) of the original iff file.</div>\n");
    fprintf(hFile, "</body>\n");
    fprintf(hFile, "</html>");
    fclose(hFile);
    return 0;
}