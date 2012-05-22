/*
    libvitaboy - Open source OpenGL TSO character animation library
    apr.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

void ReadAppearance(Appearance_t& Appearance){
    printf("\n========== Appearance ==========\n");
    Appearance.Version = VBFile.readint32();
    printf("Version: %u\n", Appearance.Version);

    ReadAsset(Appearance.Thumbnail, NOGROUP);

    Appearance.BindingCount = VBFile.readint32();
    printf("Binding count: %u\n", Appearance.BindingCount);
    Appearance.Bindings = (Asset_t*) malloc(Appearance.BindingCount * sizeof(Asset_t));
    for(unsigned i=0; i<Appearance.BindingCount; i++){
        printf("\n [Binding %u]\n", i);
        ReadAsset(Appearance.Bindings[i], NOGROUP);
    }
}