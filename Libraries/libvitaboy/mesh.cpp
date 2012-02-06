#include "libvitaboy.hpp"

void ReadMesh(Mesh_t& Mesh){
    Mesh.Version = VBFile.readint32();
    printf("========== Mesh ==========\n");
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
    printf("BindingCount: %u\n", Mesh.BindingCount);
    for(unsigned i=0; i<Mesh.BindingCount; i++){
        VBFile.readint32();
        VBFile.readint32();
        VBFile.readint32();
        VBFile.readint32();
        VBFile.readint32();
    }
    
    Mesh.TextureVertexCount = VBFile.readint32();
    printf("TextureVertexCount: %u\n", Mesh.TextureVertexCount);
    Mesh.TextureVertexData = (TextureVertex_t*) malloc(Mesh.TextureVertexCount * sizeof(TextureVertex_t));
    for(unsigned i=0; i<Mesh.TextureVertexCount; i++){
        Mesh.TextureVertexData[i].u = VBFile.readfloat();
        Mesh.TextureVertexData[i].v = 1 - VBFile.readfloat();
    }
    
    Mesh.BlendDataCount = VBFile.readint32();
    printf("BlendDataCount: %u\n", Mesh.BlendDataCount);
    for(unsigned i=0; i<Mesh.BlendDataCount; i++){
        VBFile.readint32();
        VBFile.readint32();
    }
    
    Mesh.VertexCount = VBFile.readint32();
    printf("VertexCount: %u\n", Mesh.VertexCount);
    Mesh.UnclothedVertexData = (Vertex_t*) malloc(Mesh.VertexCount * sizeof(Vertex_t));
    Mesh.ClothedVertexData = (Vertex_t*) malloc(Mesh.VertexCount * sizeof(Vertex_t));
    for(unsigned i=0; i<Mesh.VertexCount; i++){
        Mesh.UnclothedVertexData[i].x = VBFile.readfloat();
        Mesh.UnclothedVertexData[i].y = VBFile.readfloat();
        Mesh.UnclothedVertexData[i].z = VBFile.readfloat();
        Mesh.ClothedVertexData[i].x = VBFile.readfloat();
        Mesh.ClothedVertexData[i].y = VBFile.readfloat();
        Mesh.ClothedVertexData[i].z = VBFile.readfloat();
    }
}