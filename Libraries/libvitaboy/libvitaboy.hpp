/*
    libvitaboy - Open source OpenGL TSO character animation library
    libvitaboy.hpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#ifndef LIBVITABOY_HPP
#define LIBVITABOY_HPP

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <FileHandler.hpp>

/****
** Bytestream
*/

class VBFile_t {
  private:
    const uint8_t *Buffer, *Position;
    unsigned Size;

  public:
    inline void set(const void *_Buffer, unsigned _Size){
        Buffer = (const uint8_t*) _Buffer;
        Position = (const uint8_t*) _Buffer;
        Size = _Size;
    }

    inline unsigned getpos(){
        return Position-Buffer;
    }

    inline void seekto(unsigned offset){
        Position = Buffer+offset;
    }
    inline void seekahead(unsigned count){
        Position += count;
    }
    inline void seekback(unsigned count){
        Position -= count;
    }

    inline uint32_t readint32(){
        uint32_t value = (uint32_t)((Position[0]<<(8*3)) | (Position[1]<<(8*2)) | (Position[2]<<(8*1)) | (Position[3]<<(8*0)));
        Position += 4;
        return value;
    }

    inline uint32_t readint16(){
        uint16_t value = (uint16_t)((Position[0]<<(8*1)) | (Position[1]<<(8*0)));
        Position += 2;
        return value;
    }

    inline uint32_t readint8(){
        uint8_t value = (uint8_t)((Position[0]<<(8*0)));
        Position += 1;
        return value;
    }

    inline float readfloat(){
        union { uint32_t i; float f; } value;
        value.i = (uint32_t)((Position[0]<<(8*0)) | (Position[1]<<(8*1)) | (Position[2]<<(8*2)) | (Position[3]<<(8*3)));
        Position += 4;
        return value.f;
    }

    inline void readbytes(void* Destination, unsigned length){
        memcpy(Destination, Position, length);
        Position += length;
    }

    inline char* readstring(){
        //Read a Pascal string with 1 length byte
        unsigned length = readint8();
        char *string = (char*) malloc(length+1);
        readbytes(string, length);
        string[length] = '\0';
        return string;
    }

    inline char* readstring2(){
        //Read a Pascal string with 2 length bytes
        unsigned length = readint16();
        char *string = (char*) malloc(length+1);
        readbytes(string, length);
        string[length] = '\0';
        return string;
    }
};

extern VBFile_t VBFile;

/****
** Common
*/

enum ReadGroup {
    NOGROUP,
    READGROUP
};

struct Translation_t {
    float x, y, z;
};

struct Rotation_t {
    float x, y, z, w;
};

struct KeyValuePair_t {
    char * Key;
    char * Value;
};

struct Prop_t {
    uint32_t EntriesCount;
    KeyValuePair_t * Entries;
};

struct PropsList_t {
    uint32_t PropsCount;
    Prop_t * Props;
};

void ReadAsset(Asset_t& Asset, bool ReadGroup);
void ReadPropEntry(KeyValuePair_t& Entry);
void ReadPropEntries(Prop_t& Prop);
void ReadPropsList(PropsList_t& PropsList);
float DotProduct(Rotation_t * q1, Rotation_t * q2);
void Normalize(Rotation_t * q);
void CombineQuaternions(Rotation_t * Destination, Rotation_t * Source);
void FindQuaternionMatrix(float * Matrix, Rotation_t * Quaternion);


/****
** Animation (*.anim)
*/

struct TimeProp_t {
    uint32_t ID;
    PropsList_t PropsList;
};

struct TimePropsList_t {
    uint32_t TimePropsCount;
    TimeProp_t * TimeProps;
};

struct Motion_t {
    uint32_t Unknown;
    char * BoneName;
    uint32_t FrameCount;
    float Duration; //Converted to seconds
    uint8_t HasTranslation;
    uint8_t HasRotation;
    uint32_t FirstTranslation;
    uint32_t FirstRotation;
    Translation_t * Translations;
    Rotation_t * Rotations;

    uint8_t HasPropsLists;
    uint32_t PropsListsCount;
    PropsList_t * PropsLists;

    uint8_t HasTimePropsLists;
    uint32_t TimePropsListsCount;
    TimePropsList_t * TimePropsLists;
};

struct Animation_t {
    uint32_t Version;
    char * Name;
    float Duration; //Converted to seconds
    float Distance;
    uint8_t IsMoving;
    uint32_t TranslationsCount;
    uint32_t RotationsCount;
    uint32_t MotionsCount;

    unsigned TranslationsOffset;
    unsigned RotationsOffset;

    Motion_t * Motions;
};

void ReadAnimation(Animation_t& Animation);
void ReadMotion(Animation_t& Animation, Motion_t& Motion);
void ReadPropsLists(Motion_t& Motion);
void ReadTimePropsList(TimePropsList_t& TimePropsList);
void ReadTimePropsLists(Motion_t& Motion);


/****
** Appearance (*.apr)
*/

struct Appearance_t {
    uint32_t Version;
    Asset_t Thumbnail;
    uint32_t BindingCount;
    Asset_t * Bindings;
};

void ReadAppearance(Appearance_t& Appearance);


/****
** Binding (*.bnd)
*/

struct Binding_t {
    uint32_t Version;
    char * BoneName;
    uint32_t MeshDef;
    Asset_t Mesh;
    uint32_t AppearanceDef;
    Asset_t Appearance;
};

void ReadBinding(Binding_t& Binding);


/****
** Collection (*.col)
*/

struct PODef_t {
    uint32_t Index;
    Asset_t PO;
};

struct Collection_t {
    uint32_t POCount;
    PODef_t * PurchasableOutfits;
};

void ReadCollection(Collection_t& Collection);


/****
** Hand Group (*.hag)
*/

struct HandGroup_t {
    uint32_t Version;
    Asset_t HandAppearances[18];
};

void ReadHandGroup(HandGroup_t& HandGroup);


/****
** Mesh (*.mesh)
*/

struct TextureVertex_t {
    float u, v;
};

struct Coord_t {
    float x, y, z;
};

struct TextureCoord_t {
    float u, v;
};

struct NormalCoord_t {
    float x, y, z;
};

struct BlendData_t {
    float Weight;
    unsigned OtherVertex;
};

struct Vertex_t {
    Coord_t Coord;
    TextureCoord_t TextureCoord;
    NormalCoord_t NormalCoord;

    unsigned BoneIndex;
    BlendData_t BlendData;
};

struct Face_t {
    unsigned VertexA, VertexB, VertexC;
};

struct BoneBinding_t {
    unsigned BoneIndex;
    unsigned FirstRealVertex;
    unsigned RealVertexCount;
    unsigned FirstBlendVertex;
    unsigned BlendVertexCount;
};

struct Mesh_t {
    uint32_t Version;
    uint32_t BoneCount;
    char ** BoneNames;
    uint32_t FaceCount;
    Face_t * FaceData;
    uint32_t BindingCount;
    BoneBinding_t * BoneBindings;
    uint32_t RealVertexCount;
    uint32_t BlendVertexCount;
    uint32_t TotalVertexCount;
    Vertex_t * VertexData;
    Vertex_t * TransformedVertexData;
};

void ReadMesh(Mesh_t& Mesh);


/****
** Outfit (*.oft)
*/

enum OutfitColor {
    OutfitColor_Light,
    OutfitColor_Medium,
    OutfitColor_Dark
};

enum OutfitRegion {
    OutfitRegion_Head = 0,
    OutfitRegion_Body = 18
};

struct Outfit_t {
    uint32_t Version;
    uint32_t Unknown;
    Asset_t Appearance[3];
    uint32_t Group;
    uint32_t Region;
};

void ReadOutfit(Outfit_t& Outfit);


/****
** Purchasable Outfit (*.po)
*/

struct PurchasableOutfit_t {
    uint32_t Version;
    uint32_t Unknown;
    uint32_t OutfitDef;
    Asset_t Outfit;
    uint32_t CollectionDef;
    Asset_t Collection;
};

void ReadPurchasableOutfit(PurchasableOutfit_t& PurchasableOutfit);


/****
** Skeleton (*.skel)
*/

struct Bone_t {
    uint32_t Unknown;
    char * Name;
    char * ParentsName;
    uint8_t HasProps;
    PropsList_t PropsList;
    Translation_t Translation;
    Rotation_t Rotation;
    uint32_t CanTranslate;
    uint32_t CanRotate;
    uint32_t CanBlend;
    float WiggleValue;
    float WigglePower;

    unsigned ChildrenCount;
    Bone_t ** Children;
};

struct Skeleton_t {
    uint32_t Version;
    char * Name;
    uint16_t BoneCount;
    Bone_t * Bones;
};

void ReadSkeleton(Skeleton_t& Bone);
void ReadBone(Skeleton_t& Skeleton, Bone_t& Bone, unsigned Index);
unsigned FindBone(Skeleton_t& Skeleton, const char * BoneName, unsigned Count);

#endif