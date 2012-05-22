/*
    libvitaboy - Open source OpenGL TSO character animation library
    oft.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include "libvitaboy.hpp"

void ReadOutfit(Outfit_t& Outfit){
    printf("\n========== Outfit ==========\n");
    Outfit.Version = VBFile.readint32();
    printf("Version: %u\n", Outfit.Version);

    Outfit.Unknown = VBFile.readint32();
    printf("Unknown: %u\n", Outfit.Unknown);

    const char* Colors[] = {"Light", "Medium", "Dark"};
    for(unsigned i=0; i<3; i++){
        printf("\n [%s Appearance]\n", Colors[i]);
        ReadAsset(Outfit.Appearance[i], NOGROUP);
    }

    Outfit.Group = VBFile.readint32();
    printf("Group: %u\n", Outfit.Group);
    Outfit.Region = VBFile.readint32();
    printf("Region: %u\n", Outfit.Region);
}