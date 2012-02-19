/*
    libvitaboy - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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
        Mesh.BoneBindings[i].FirstFixedVertex = VBFile.readint32();
        Mesh.BoneBindings[i].FixedVertexCount = VBFile.readint32();
        Mesh.BoneBindings[i].FirstBlendedVertex = VBFile.readint32();
        Mesh.BoneBindings[i].BlendedVertexCount = VBFile.readint32();
    }
    
    Mesh.FixedVertexCount = VBFile.readint32();
    printf("FixedVertexCount: %u\n", Mesh.FixedVertexCount);
    Mesh.TextureVertexData = (TextureVertex_t*) malloc(Mesh.FixedVertexCount * sizeof(TextureVertex_t));
    for(unsigned i=0; i<Mesh.FixedVertexCount; i++){
        Mesh.TextureVertexData[i].u = VBFile.readfloat();
        Mesh.TextureVertexData[i].v = VBFile.readfloat();
    }
    
    Mesh.BlendedVertexCount = VBFile.readint32();
    printf("BlendedVertexCount: %u\n", Mesh.BlendedVertexCount);
    Mesh.BlendData = (BlendData_t*) malloc(Mesh.BlendedVertexCount * sizeof(BlendData_t));
    for(unsigned i=0; i<Mesh.BlendedVertexCount; i++){
        Mesh.BlendData[i].Weight = (float)VBFile.readint32()/0x8000;
        Mesh.BlendData[i].OtherVertex = VBFile.readint32();
    }
    
    Mesh.TotalVertexCount = VBFile.readint32();
    printf("TotalVertexCount: %u\n", Mesh.TotalVertexCount);
    Mesh.VertexData = (Vertex_t*) malloc(Mesh.TotalVertexCount * sizeof(Vertex_t));
    Mesh.VertexNorms = (Vertex_t*) malloc(Mesh.TotalVertexCount * sizeof(Vertex_t));
    Mesh.TransformedVertexData = (Vertex_t*) malloc(Mesh.TotalVertexCount * sizeof(Vertex_t));
    Mesh.TransformedVertexNorms = (Vertex_t*) malloc(Mesh.TotalVertexCount * sizeof(Vertex_t));
    Mesh.TransformedTextureData = (TextureVertex_t*) malloc(Mesh.TotalVertexCount * sizeof(Vertex_t));
    for(unsigned i=0; i<Mesh.TotalVertexCount; i++){
        Mesh.VertexData[i].x = VBFile.readfloat();
        Mesh.VertexData[i].y = VBFile.readfloat();
        Mesh.VertexData[i].z = VBFile.readfloat();
        Mesh.VertexNorms[i].x = VBFile.readfloat();
        Mesh.VertexNorms[i].y = VBFile.readfloat();
        Mesh.VertexNorms[i].z = VBFile.readfloat();
        
        if(i<Mesh.FixedVertexCount){
            //Fixed vertex
            Mesh.TransformedTextureData[i].u = Mesh.TextureVertexData[i].u;
            Mesh.TransformedTextureData[i].v = Mesh.TextureVertexData[i].v;
        }else{
            //Blended vertex; inherit
            TextureVertex_t& Parent = Mesh.TextureVertexData[Mesh.BlendData[i-Mesh.FixedVertexCount].OtherVertex];
            Mesh.TransformedTextureData[i].u = Parent.u;
            Mesh.TransformedTextureData[i].v = Parent.v;
        }
    }
}