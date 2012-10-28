/*
    libvitaboy - Open source OpenGL TSO character animation library
    mesh.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

void ReadMesh(Mesh_t& Mesh){
    printf("\n========== Mesh ==========\n");
    Mesh.Version = VBFile.readint32();
    printf("Version: %u\n", Mesh.Version);

    Mesh.BoneCount = VBFile.readint32();
    printf("BoneCount: %u\n", Mesh.BoneCount);
    Mesh.BoneNames = (char**) malloc(Mesh.BoneCount * sizeof(char*));
    for(unsigned i=0; i<Mesh.BoneCount; i++){
        Mesh.BoneNames[i] = VBFile.readstring();
        printf("| Bone %u: %s\n", i+1, Mesh.BoneNames[i]);
    }

    Mesh.FaceCount = VBFile.readint32();
    printf("FaceCount: %u\n", Mesh.FaceCount);
    Mesh.FaceData = (Face_t*) malloc(Mesh.FaceCount * sizeof(Face_t));
    for(unsigned i=0; i<Mesh.FaceCount; i++){
        Mesh.FaceData[i].VertexA = VBFile.readint32();
        Mesh.FaceData[i].VertexB = VBFile.readint32();
        Mesh.FaceData[i].VertexC = VBFile.readint32();
    }

    Mesh.BindingCount = VBFile.readint32();
    Mesh.BoneBindings = (BoneBinding_t*) malloc(Mesh.BindingCount * sizeof(BoneBinding_t));
    printf("BindingCount: %u\n", Mesh.BindingCount);
    for(unsigned i=0; i<Mesh.BindingCount; i++){
        Mesh.BoneBindings[i].BoneIndex = VBFile.readint32();
        Mesh.BoneBindings[i].FirstRealVertex = VBFile.readint32();
        Mesh.BoneBindings[i].RealVertexCount = VBFile.readint32();
        Mesh.BoneBindings[i].FirstBlendVertex = VBFile.readint32();
        Mesh.BoneBindings[i].BlendVertexCount = VBFile.readint32();
    }

    Mesh.RealVertexCount = VBFile.readint32();
    printf("RealVertexCount: %u\n", Mesh.RealVertexCount);
    TextureVertex_t * TextureVertexData = (TextureVertex_t*) malloc(Mesh.RealVertexCount * sizeof(TextureVertex_t));
    for(unsigned i=0; i<Mesh.RealVertexCount; i++){
        TextureVertexData[i].u = VBFile.readfloat();
        TextureVertexData[i].v = -VBFile.readfloat();
    }

    Mesh.BlendVertexCount = VBFile.readint32();
    printf("BlendVertexCount: %u\n", Mesh.BlendVertexCount);
    BlendData_t * BlendData = (BlendData_t*) malloc(Mesh.BlendVertexCount * sizeof(BlendData_t));
    for(unsigned i=0; i<Mesh.BlendVertexCount; i++){
        BlendData[i].Weight = (float)VBFile.readint32()/0x8000;
        BlendData[i].OtherVertex = VBFile.readint32();
    }

    Mesh.TotalVertexCount = VBFile.readint32();
    printf("TotalVertexCount: %u\n", Mesh.TotalVertexCount);
    Mesh.VertexData = (Vertex_t*) malloc(Mesh.TotalVertexCount * sizeof(Vertex_t));
    Mesh.TransformedVertexData = (Vertex_t*) malloc(Mesh.TotalVertexCount * sizeof(Vertex_t));
    for(unsigned i=0; i<Mesh.TotalVertexCount; i++){
        Mesh.VertexData[i].Coord.x = -VBFile.readfloat();
        Mesh.VertexData[i].Coord.y = VBFile.readfloat();
        Mesh.VertexData[i].Coord.z = VBFile.readfloat();
        Mesh.TransformedVertexData[i].NormalCoord.x = -VBFile.readfloat();
        Mesh.TransformedVertexData[i].NormalCoord.y = VBFile.readfloat();
        Mesh.TransformedVertexData[i].NormalCoord.z = VBFile.readfloat();

        if(i<Mesh.RealVertexCount){
            //Fixed vertex
            Mesh.TransformedVertexData[i].TextureCoord.u = TextureVertexData[i].u;
            Mesh.TransformedVertexData[i].TextureCoord.v = TextureVertexData[i].v;
        }else{
            //Blended vertex
            Mesh.TransformedVertexData[i].BlendData.Weight      = BlendData[i-Mesh.RealVertexCount].Weight;
            Mesh.TransformedVertexData[i].BlendData.OtherVertex = BlendData[i-Mesh.RealVertexCount].OtherVertex;
        }
    }

    free(TextureVertexData);
    free(BlendData);
}