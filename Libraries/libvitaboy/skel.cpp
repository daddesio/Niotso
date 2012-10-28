/*
    libvitaboy - Open source OpenGL TSO character animation library
    skel.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

void ReadSkeleton(Skeleton_t& Skeleton){
    printf("\n========== Skeleton ==========\n");
    Skeleton.Version = VBFile.readint32();
    printf("Version: %u\n", Skeleton.Version);
    Skeleton.Name = VBFile.readstring();
    printf("Name: %s\n", Skeleton.Name);

    Skeleton.BoneCount = VBFile.readint16();
    printf("BoneCount: %u\n", Skeleton.BoneCount);
    Skeleton.Bones = (Bone_t*) malloc(Skeleton.BoneCount * sizeof(Bone_t));
    for(unsigned i=0; i<Skeleton.BoneCount; i++){
        printf("\n [Bone %u]\n", i);
        ReadBone(Skeleton, Skeleton.Bones[i], i);
    }
}

void ReadBone(Skeleton_t& Skeleton, Bone_t& Bone, unsigned Index){
    Bone.Unknown = VBFile.readint32();
    printf(" | Unknown: %u\n", Bone.Unknown);
    Bone.Name = VBFile.readstring();
    printf(" | Name: %s\n", Bone.Name);
    Bone.ParentsName = VBFile.readstring();
    printf(" | Parent's name: %s\n", Bone.ParentsName);

    Bone.HasProps = VBFile.readint8();
    printf(" | HasProps: %u\n", Bone.HasProps);
    if(Bone.HasProps){
        ReadPropsList(Bone.PropsList);
    }

    printf(" | Translation:\n");
    Bone.Translation.x = -VBFile.readfloat();
    printf(" | | x: %g\n", Bone.Translation.x);
    Bone.Translation.y = VBFile.readfloat();
    printf(" | | y: %g\n", Bone.Translation.y);
    Bone.Translation.z = VBFile.readfloat();
    printf(" | | z: %g\n", Bone.Translation.z);
    printf(" | Rotation:\n");
    Bone.Rotation.x = VBFile.readfloat();
    printf(" | | x: %g\n", Bone.Rotation.x);
    Bone.Rotation.y = -VBFile.readfloat();
    printf(" | | y: %g\n", Bone.Rotation.y);
    Bone.Rotation.z = -VBFile.readfloat();
    printf(" | | z: %g\n", Bone.Rotation.z);
    Bone.Rotation.w = VBFile.readfloat();
    printf(" | | w: %g\n", Bone.Rotation.w);

    Bone.CanTranslate = VBFile.readint32();
    printf(" | CanTranslate: %u\n", Bone.CanTranslate);
    Bone.CanRotate = VBFile.readint32();
    printf(" | CanRotate: %u\n", Bone.CanRotate);
    Bone.CanBlend = VBFile.readint32();
    printf(" | CanBlend: %u\n", Bone.CanBlend);

    Bone.WiggleValue = VBFile.readfloat();
    printf(" | WiggleValue: %g\n", Bone.WiggleValue);
    Bone.WigglePower = VBFile.readfloat();
    printf(" | WigglePower: %g\n", Bone.WigglePower);

    Bone.ChildrenCount = 0;
    Bone.Children = (Bone_t**) malloc((Skeleton.BoneCount-Index-1) * sizeof(Bone_t*));

    unsigned Parent = FindBone(Skeleton, Bone.ParentsName, Index);
    if(Parent != (unsigned)-1){
        Bone_t& ParentBone = Skeleton.Bones[Parent];
        ParentBone.Children[ParentBone.ChildrenCount] = &Bone;
        ParentBone.ChildrenCount++;
    }
}

unsigned FindBone(Skeleton_t& Skeleton, const char * BoneName, unsigned Count){
    for(unsigned i=0; i<Count; i++){
        if(!strcmp(Skeleton.Bones[i].Name, BoneName)) return i;
    }

    return (unsigned)-1;
}