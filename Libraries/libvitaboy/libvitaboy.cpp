/*
    libvitaboy - Open source OpenGL TSO character animation library
    libvitaboy.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include <math.h>
#include "libvitaboy.hpp"

VBFile_t VBFile;

void ReadAsset(Asset_t& Asset, bool ReadGroup){
    Asset.Group = (ReadGroup) ? VBFile.readint32() : 0xA96F6D42;
    printf(" | Group: %u\n", Asset.Group);
    Asset.File = VBFile.readint32();
    printf(" | File: %u\n", Asset.File);
    Asset.Type = VBFile.readint32();
    printf(" | Type: %u\n", Asset.Type);
}

void ReadPropEntry(KeyValuePair_t& Entry){
    Entry.Key = VBFile.readstring();
    printf(" | | | | | Key: %s\n", Entry.Key);
    Entry.Value = VBFile.readstring();
    printf(" | | | | | Value: %s\n", Entry.Value);
}

void ReadPropEntries(Prop_t& Prop){
    unsigned count = Prop.EntriesCount = VBFile.readint32();
    printf(" | | | | EntriesCount: %u\n", Prop.EntriesCount);
    Prop.Entries = (KeyValuePair_t*) malloc(count * sizeof(KeyValuePair_t));

    for(unsigned i=0; i<count; i++){
        printf(" | | | | [Entry %u]\n", i+1);
        ReadPropEntry(Prop.Entries[i]);
    }
}

float DotProduct(Rotation_t * q1, Rotation_t * q2){
    return q1->x*q2->x + q1->y*q2->y + q1->z*q2->z + q1->w*q2->w;
}

void Normalize(Rotation_t * q){
    float magnitude = q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w;
    if(magnitude != 0){
        magnitude = 1.0f/sqrt(magnitude);
        q->x *= magnitude;
        q->y *= magnitude;
        q->z *= magnitude;
        q->w *= magnitude;
    }
}

void FindQuaternionMatrix(float * Matrix, Rotation_t * Quaternion){
	float x2 = Quaternion->x * Quaternion->x;
	float y2 = Quaternion->y * Quaternion->y;
	float z2 = Quaternion->z * Quaternion->z;
	float xy = Quaternion->x * Quaternion->y;
	float xz = Quaternion->x * Quaternion->z;
	float yz = Quaternion->y * Quaternion->z;
	float wx = Quaternion->w * Quaternion->x;
	float wy = Quaternion->w * Quaternion->y;
	float wz = Quaternion->w * Quaternion->z;

    Matrix[0]  = 1.0f - 2.0f * (y2 + z2);
    Matrix[1]  = 2.0f * (xy - wz);
    Matrix[2]  = 2.0f * (xz + wy);
    Matrix[3]  = 0.0f;
    Matrix[4]  = 2.0f * (xy + wz);
    Matrix[5]  = 1.0f - 2.0f * (x2 + z2);
    Matrix[6]  = 2.0f * (yz - wx);
    Matrix[7]  = 0.0f;
    Matrix[8]  = 2.0f * (xz - wy);
    Matrix[9]  = 2.0f * (yz + wx);
    Matrix[10] = 1.0f - 2.0f * (x2 + y2);
    Matrix[11] = 0.0f;
    Matrix[12] = 0.0f;
    Matrix[13] = 0.0f;
    Matrix[14] = 0.0f;
    Matrix[15] = 1.0f;
}
