/*
    libvitaboy - Open source OpenGL TSO character animation library
    col.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

void ReadCollection(Collection_t& Collection){
    printf("\n========== Collection ==========\n");
    Collection.POCount = VBFile.readint32();
    printf("Purchasable Outfit count: %u\n", Collection.POCount);
    Collection.PurchasableOutfits = (PODef_t*) malloc(Collection.POCount * sizeof(PODef_t));
    for(unsigned i=0; i<Collection.POCount; i++){
        printf("\n [Purchasable Outfit %u]\n", i);

        Collection.PurchasableOutfits[i].Index = VBFile.readint32();
        printf(" | Index: %u\n", Collection.PurchasableOutfits[i].Index);
        ReadAsset(Collection.PurchasableOutfits[i].PO, NOGROUP);
    }
}