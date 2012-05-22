/*
    libvitaboy - Open source OpenGL TSO character animation library
    po.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

void ReadPurchasableOutfit(PurchasableOutfit_t& PurchasableOutfit){
    printf("\n========== Purchasable Outfit ==========\n");
    PurchasableOutfit.Version = VBFile.readint32();
    printf("Version: %u\n", PurchasableOutfit.Version);

    PurchasableOutfit.Unknown = VBFile.readint32();
    printf("Unknown: %u\n", PurchasableOutfit.Unknown);

    PurchasableOutfit.OutfitDef = VBFile.readint32();
    if(PurchasableOutfit.OutfitDef){
        printf("\n Outfit:\n");
        ReadAsset(PurchasableOutfit.Outfit, READGROUP);
    }

    PurchasableOutfit.CollectionDef = VBFile.readint32();
    if(PurchasableOutfit.CollectionDef){
        printf("\n Collection:\n");
        ReadAsset(PurchasableOutfit.Collection, READGROUP);
    }
}