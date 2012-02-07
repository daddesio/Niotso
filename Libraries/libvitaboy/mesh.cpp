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
        Mesh.BoneBindings[i].FirstVertex = VBFile.readint32();
        Mesh.BoneBindings[i].VertexCount = VBFile.readint32();
        Mesh.BoneBindings[i].FirstBlendedVertex = VBFile.readint32();
        Mesh.BoneBindings[i].BlendedVertexCount = VBFile.readint32();
    }
    
    Mesh.TextureVertexCount = VBFile.readint32();
    printf("TextureVertexCount: %u\n", Mesh.TextureVertexCount);
    Mesh.TextureVertexData = (TextureVertex_t*) malloc(Mesh.TextureVertexCount * sizeof(TextureVertex_t));
    for(unsigned i=0; i<Mesh.TextureVertexCount; i++){
        Mesh.TextureVertexData[i].u = VBFile.readfloat();
        Mesh.TextureVertexData[i].v = VBFile.readfloat();
    }
    
    Mesh.BlendDataCount = VBFile.readint32();
    printf("BlendDataCount: %u\n", Mesh.BlendDataCount);
    for(unsigned i=0; i<Mesh.BlendDataCount; i++){
        VBFile.readint32();
        VBFile.readint32();
    }
    
    Mesh.VertexCount = VBFile.readint32();
    printf("VertexCount: %u\n", Mesh.VertexCount);
    Mesh.VertexData = (Vertex_t*) malloc(Mesh.VertexCount * sizeof(Vertex_t));
    Mesh.VertexNorms = (Vertex_t*) malloc(Mesh.VertexCount * sizeof(Vertex_t));
    for(unsigned i=0; i<Mesh.VertexCount; i++){
        Mesh.VertexData[i].x = VBFile.readfloat();
        Mesh.VertexData[i].y = VBFile.readfloat();
        Mesh.VertexData[i].z = VBFile.readfloat();
        Mesh.VertexNorms[i].x = VBFile.readfloat();
        Mesh.VertexNorms[i].y = VBFile.readfloat();
        Mesh.VertexNorms[i].z = VBFile.readfloat();
    }
}