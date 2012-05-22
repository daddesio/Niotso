/*
    hitutils - The Sims HIT (dis)assembler and linker
    hitdump.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include <stdio.h>

int main(){
    printf("Usage: hitdump [-f] [-o outfile.txt] [-hsm outfile.hsm]\n"
    "       [-hot outfile.hot] infile.hit\n"
    "Disassemble a HIT binary.\n"
    "\n"
    "The HSM and HOT files associated with the HIT file are required\n"
    "as inputs; their paths default to same base name as the input\n"
    "file but can be changed with the above options.\n"
    "Use -f to force overwriting without confirmation.\n"
    "\n"
    "Report bugs to <X-Fi6@phppoll.org>.\n"
    "hitutils is maintained by the Niotso project.\n"
    "Home page: <http://www.niotso.org/>\n");
    return 0;
}