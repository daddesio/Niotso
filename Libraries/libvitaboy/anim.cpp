/*
    libvitaboy - Open source OpenGL TSO character animation library
    anim.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

static unsigned motionnumber = 0;

void ReadAnimation(Animation_t& Animation){
    printf("===== Animation =====\n");
    Animation.Version = VBFile.readint32();
    printf("Version: %u\n", Animation.Version);
    Animation.Name = VBFile.readstring2();
    printf("Name: %s\n", Animation.Name);
    Animation.Duration = VBFile.readfloat();
    printf("Duration: %g\n", Animation.Duration/1000);
    Animation.Distance = VBFile.readfloat();
    printf("Distance: %g\n", Animation.Distance);
    Animation.IsMoving = VBFile.readint8();
    printf("IsMoving: %u\n", Animation.IsMoving);

    Animation.TranslationsCount = VBFile.readint32();
    printf("TranslationsCount: %u\n", Animation.TranslationsCount);
    Animation.TranslationsOffset = VBFile.getpos();
    VBFile.seekto(Animation.TranslationsOffset + 12*Animation.TranslationsCount);

    Animation.RotationsCount = VBFile.readint32();
    printf("RotationsCount: %u\n", Animation.RotationsCount);
    Animation.RotationsOffset = VBFile.getpos();
    VBFile.seekto(Animation.RotationsOffset + 16*Animation.RotationsCount);

    Animation.MotionsCount = VBFile.readint32();
    printf("MotionsCount: %u\n", Animation.MotionsCount);
    Animation.Motions = (Motion_t*) malloc(Animation.MotionsCount * sizeof(Motion_t));
    for(unsigned i=0; i<Animation.MotionsCount; i++){
        ReadMotion(Animation, Animation.Motions[i]);
    }
}

void ReadMotion(Animation_t& Animation, Motion_t& Motion){
    motionnumber++;
    printf("\n\n [Motion %u]\n", motionnumber);
    Motion.Unknown = VBFile.readint32();
    printf(" | Unknown: %u\n", Motion.Unknown);
    Motion.BoneName = VBFile.readstring();
    printf(" | BoneName: %s\n", Motion.BoneName);
    Motion.FrameCount = VBFile.readint32();
    printf(" | FrameCount: %u\n", Motion.FrameCount);
    Motion.Duration = VBFile.readfloat();
    printf(" | Duration: %g\n", Motion.Duration/1000);

    Motion.HasTranslation = VBFile.readint8();
    printf(" | HasTranslation: %u\n", Motion.HasTranslation);
    Motion.HasRotation = VBFile.readint8();
    printf(" | HasRotation: %u\n", Motion.HasRotation);
    Motion.FirstTranslation = VBFile.readint32();
    if(Motion.HasTranslation)
        printf(" | FirstTranslation: %u\n", Motion.FirstTranslation);
    Motion.FirstRotation = VBFile.readint32();
    if(Motion.HasRotation)
        printf(" | FirstRotation: %u\n", Motion.FirstRotation);

    if(Motion.HasTranslation){
        Motion.Translations = (Translation_t*) malloc(Motion.FrameCount * sizeof(Translation_t));

        unsigned pos = VBFile.getpos();
        VBFile.seekto(Animation.TranslationsOffset + 12*Motion.FirstTranslation);
        for(unsigned i=0; i<Motion.FrameCount; i++){
            Motion.Translations[i].x = -VBFile.readfloat();
            Motion.Translations[i].y = VBFile.readfloat();
            Motion.Translations[i].z = VBFile.readfloat();
        }
        VBFile.seekto(pos);
    }

    if(Motion.HasRotation){
        Motion.Rotations = (Rotation_t*) malloc(Motion.FrameCount * sizeof(Rotation_t));

        unsigned pos = VBFile.getpos();
        VBFile.seekto(Animation.RotationsOffset + 16*Motion.FirstRotation);
        for(unsigned i=0; i<Motion.FrameCount; i++){
            Motion.Rotations[i].x = VBFile.readfloat();
            Motion.Rotations[i].y = -VBFile.readfloat();
            Motion.Rotations[i].z = -VBFile.readfloat();
            Motion.Rotations[i].w = VBFile.readfloat();
        }
        VBFile.seekto(pos);
    }

    Motion.HasPropsLists = VBFile.readint8();
    printf(" | HasPropsLists: %u\n", Motion.HasPropsLists);
    if(Motion.HasPropsLists){
        ReadPropsLists(Motion);
    }

    Motion.HasTimePropsLists = VBFile.readint8();
    printf(" | HasTimePropsLists: %u\n", Motion.HasTimePropsLists);
    if(Motion.HasTimePropsLists){
        ReadTimePropsLists(Motion);
    }
}

void ReadPropsList(PropsList_t& PropsList){
    unsigned count = PropsList.PropsCount = VBFile.readint32();
    printf(" | | | PropsCount: %u\n", count);
    PropsList.Props = (Prop_t*) malloc(count * sizeof(Prop_t));

    for(unsigned i=0; i<count; i++){
        printf(" | | | [Prop %u]\n", i+1);
        ReadPropEntries(PropsList.Props[i]);
    }
}

void ReadPropsLists(Motion_t& Motion){
    unsigned count = Motion.PropsListsCount = VBFile.readint32();
    Motion.PropsLists = (PropsList_t*) malloc(count * sizeof(PropsList_t));

    for(unsigned i=0; i<count; i++){
        ReadPropsList(Motion.PropsLists[i]);
    }
}

void ReadTimePropsList(TimePropsList_t& TimePropsList){
    unsigned count = TimePropsList.TimePropsCount = VBFile.readint32();
    printf(" | | TimePropsCount: %u\n", count);
    TimePropsList.TimeProps = (TimeProp_t*) malloc(count * sizeof(TimeProp_t));

    for(unsigned i=0; i<count; i++){
        printf(" | | [TimeProp %u]\n", i+1);
        TimePropsList.TimeProps[i].ID = VBFile.readint32();
        printf(" | | | ID: %u\n", TimePropsList.TimeProps[i].ID);
        ReadPropsList(TimePropsList.TimeProps[i].PropsList);
    }
}

void ReadTimePropsLists(Motion_t& Motion){
    unsigned count = Motion.TimePropsListsCount = VBFile.readint32();
    printf(" | TimePropsListsCount: %u\n", count);
    Motion.TimePropsLists = (TimePropsList_t*) malloc(count * sizeof(TimePropsList_t));

    for(unsigned i=0; i<count; i++){
        printf(" | [TimePropsList %u]\n", i+1);
        ReadTimePropsList(Motion.TimePropsLists[i]);
    }
}