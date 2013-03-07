/*
    libvitaboy - Open source OpenGL TSO character animation library
    Renderer.cpp - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

/*
    Instructions:
    You must have the following files in the same directory as the Renderer binary:

    Skeleton:
    * skeleton.skel ("adult.skel" in ./avatardata/skeletons/)

    Meshes:
    * body.mesh (pick one from ./avatardata/bodies/meshes/)
    * head.mesh (pick one from ./avatardata/heads/meshes/)
    * lhand.mesh (pick one from ./avatardata/hands/meshes/)
    * rhand.mesh (pick one from ./avatardata/hands/meshes/)

    Textures:
    * body.jpg (pick one from ./avatardata/bodies/textures/)
    * head.jpg (pick one from ./avatardata/heads/textures/)
    * hand.jpg (pick one from ./avatardata/hands/textures/)

    Animation:
    * animation.anim (pick one from ./avatardata/animations/)

    ==== Controls ====
    Arrow keys: Rotate the sim
    i,j,k,l: Translate the sim around the screen
    z,x: Rotate the sim like a clock
    a,s: Zoom in, out
    q: Toggle skeleton
    n: Animate the character
    F11: Enter/leave fullscreen
*/

#include <math.h>
#include <FileHandler.hpp>
#include <libgldemo.h>
#include "libvitaboy.hpp"

static float zoom = -10;
struct BasicVertex_t {
    float x, y, z;
};
struct CharacterPlacement_t {
    BasicVertex_t Translation;
    BasicVertex_t Rotation;
};
static CharacterPlacement_t Character = {{0,-3,0}, {0,0,0}};

static Skeleton_t Skeleton;

static const unsigned TextureCount = 3;
static unsigned texture[3];
enum { Texture_Body, Texture_Head, Texture_Hand };
static const char* const TexturePaths[] = {"body.jpg", "head.jpg", "hand.jpg"};

static const unsigned MeshCount = 4;
static Mesh_t Meshes[4];
enum { Mesh_Body, Mesh_Head, Mesh_LHand, Mesh_RHand };
static const char* const MeshPaths[]    = {"body.mesh", "head.mesh", "lhand.mesh", "rhand.mesh" };
static const unsigned Mesh_UseTexture[] = { Texture_Body, Texture_Head, Texture_Hand, Texture_Hand };
static const char* const MeshActivate[] = {NULL, "HEAD", "L_HAND", "R_HAND"};

static Animation_t Animation;
static float AnimationTime = 0;

static bool ShowMesh = true;
static bool ShowSkeleton = true;

static bool PressedQ = false;

static void DisplayFileError(const char * Filename){
    const char * Message;
    switch(File::Error){
    case FERR_NOT_FOUND:
        Message = "%s does not exist.";
        break;
    case FERR_OPEN:
        Message = "%s could not be opened for reading.";
        break;
    case FERR_BLANK:
    case FERR_UNRECOGNIZED:
    case FERR_INVALIDDATA:
        Message = "%s is corrupt or invalid.";
        break;
    case FERR_MEMORY:
        Message = "Memory for %s could not be allocated.";
        break;
    default:
        Message = "%s could not be read.";
        break;
    }

    char Buffer[1024];
    sprintf(Buffer, Message, Filename);
    DemoErrorBox(Buffer);
}

static int LoadTextures()
{
    glGenTextures(3, texture);
    for(int i=0; i<3; i++){
        Image_t * Image = File::ReadImageFile(TexturePaths[i]);
        if(!Image){
            DisplayFileError(TexturePaths[i]);
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->Width, Image->Height, 0, GL_BGR, GL_UNSIGNED_BYTE, Image->Data);
        free(Image->Data);
        free(Image);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    return 1;
}

static int InitGL()
{
    if(!LoadTextures())
        return false;

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_RESCALE_NORMAL);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glFrontFace(GL_CW);
    return 1;
}

static int ResizeScene(uint16_t width, uint16_t height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    // glScalef(-1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    return 1;
}

static void TransformVertices(Bone_t& Bone)
{
    glTranslatef(Bone.Translation.x, Bone.Translation.y, Bone.Translation.z);
    float Matrix[16];
    FindQuaternionMatrix(Matrix, &Bone.Rotation);
    glMultMatrixf(Matrix);

    unsigned MeshIndex = 0;
    unsigned BoneIndex;

    for(unsigned i=1; i<MeshCount; i++){
        if(!strcmp(Bone.Name, MeshActivate[i])){
            MeshIndex = i;
            break;
        }
    }
    Mesh_t& Mesh = Meshes[MeshIndex];
    for(BoneIndex=0; BoneIndex<Mesh.BindingCount; BoneIndex++){
        if(!strcmp(Bone.Name, Mesh.BoneNames[Mesh.BoneBindings[BoneIndex].BoneIndex]))
            break;
    }

    if(BoneIndex < Mesh.BindingCount){
        for(unsigned i=0; i<Mesh.BoneBindings[BoneIndex].RealVertexCount; i++){
            unsigned VertexIndex = Mesh.BoneBindings[BoneIndex].FirstRealVertex + i;
            Vertex_t& RelativeVertex = Mesh.VertexData[VertexIndex];
            Vertex_t& AbsoluteVertex = Mesh.TransformedVertexData[VertexIndex];

            glTranslatef(RelativeVertex.Coord.x, RelativeVertex.Coord.y, RelativeVertex.Coord.z);
            glGetFloatv(GL_MODELVIEW_MATRIX, Matrix);
            AbsoluteVertex.Coord.x = Matrix[12];
            AbsoluteVertex.Coord.y = Matrix[13];
            AbsoluteVertex.Coord.z = Matrix[14];
            glTranslatef(-RelativeVertex.Coord.x, -RelativeVertex.Coord.y, -RelativeVertex.Coord.z);
        }
        for(unsigned i=0; i<Mesh.BoneBindings[BoneIndex].BlendVertexCount; i++){
            unsigned VertexIndex = Mesh.RealVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendVertex + i;
            Vertex_t& RelativeVertex = Mesh.VertexData[VertexIndex];
            Vertex_t& AbsoluteVertex = Mesh.TransformedVertexData[VertexIndex];

            glTranslatef(RelativeVertex.Coord.x, RelativeVertex.Coord.y, RelativeVertex.Coord.z);
            glGetFloatv(GL_MODELVIEW_MATRIX, Matrix);
            AbsoluteVertex.Coord.x = Matrix[12];
            AbsoluteVertex.Coord.y = Matrix[13];
            AbsoluteVertex.Coord.z = Matrix[14];
            glTranslatef(-RelativeVertex.Coord.x, -RelativeVertex.Coord.y, -RelativeVertex.Coord.z);
        }
    }

    if(Bone.ChildrenCount == 1){
        TransformVertices(*Bone.Children[0]);
    }else if(Bone.ChildrenCount > 1){
        for(unsigned i=0; i<Bone.ChildrenCount; i++){
            glPushMatrix();
            TransformVertices(*Bone.Children[i]);
            glPopMatrix();
        }
    }
}

static void BlendVertices()
{
    for(unsigned i=0; i<MeshCount; i++){
        Mesh_t& Mesh = Meshes[i];
        for(unsigned i=0; i<Mesh.BlendVertexCount; i++){
            Vertex_t& BlendVertex = Mesh.TransformedVertexData[Mesh.RealVertexCount + i];
            float Weight = BlendVertex.BlendData.Weight;
            Vertex_t& RealVertex = Mesh.TransformedVertexData[BlendVertex.BlendData.OtherVertex];
            RealVertex.Coord.x =
                Weight     * BlendVertex.Coord.x +
                (1-Weight) * RealVertex.Coord.x;
            RealVertex.Coord.y =
                Weight     * BlendVertex.Coord.y +
                (1-Weight) * RealVertex.Coord.y;
            RealVertex.Coord.z =
                Weight     * BlendVertex.Coord.z +
                (1-Weight) * RealVertex.Coord.z;
        }
    }
}

static void DrawMeshes()
{
    glPointSize(2.0);
    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glLoadIdentity();
    TransformVertices(Skeleton.Bones[0]);
    glPopMatrix();
    BlendVertices();

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    for(unsigned i=0; i<MeshCount; i++){
        glBindTexture(GL_TEXTURE_2D, texture[Mesh_UseTexture[i]]);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex_t), &Meshes[i].TransformedVertexData[0].Coord);
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_t), &Meshes[i].TransformedVertexData[0].TextureCoord);
        glDrawElements(GL_TRIANGLES, Meshes[i].FaceCount*3, GL_UNSIGNED_INT, Meshes[i].FaceData);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

static void AdvanceFrame(Skeleton_t& Skeleton, Animation_t& Animation, float TimeDelta)
{
    float Duration = (float)Animation.Motions[0].FrameCount/30;
    AnimationTime += TimeDelta;
    AnimationTime = fmodf(AnimationTime, Duration); //Loop the animation

    for(unsigned i=0; i<Animation.MotionsCount; i++){
        unsigned BoneIndex = FindBone(Skeleton, Animation.Motions[i].BoneName, Skeleton.BoneCount);
        if(BoneIndex == (unsigned)-1) continue;

        Bone_t& Bone = Skeleton.Bones[BoneIndex];

        unsigned Frame = AnimationTime*30;
        float FractionShown = AnimationTime*30 - Frame;
        unsigned NextFrame = (Frame+1 != Animation.Motions[0].FrameCount) ? Frame+1 : 0;

        if(Animation.Motions[i].HasTranslation){
            Translation_t& Translation = Animation.Motions[i].Translations[Frame];
            Translation_t& NextTranslation = Animation.Motions[i].Translations[NextFrame];
            Bone.Translation.x = (1-FractionShown)*Translation.x + FractionShown*NextTranslation.x;
            Bone.Translation.y = (1-FractionShown)*Translation.y + FractionShown*NextTranslation.y;
            Bone.Translation.z = (1-FractionShown)*Translation.z + FractionShown*NextTranslation.z;
        }
        if(Animation.Motions[i].HasRotation){
            Rotation_t& Rotation = Animation.Motions[i].Rotations[Frame];
            Rotation_t& NextRotation = Animation.Motions[i].Rotations[NextFrame];

            //Use nlerp to interpolate
            float w1 = 1.0f - FractionShown, w2 = FractionShown;
            if(DotProduct(&Rotation, &NextRotation) < 0)
                w1 *= -1;

            Bone.Rotation.x = w1*Rotation.x + w2*NextRotation.x;
            Bone.Rotation.y = w1*Rotation.y + w2*NextRotation.y;
            Bone.Rotation.z = w1*Rotation.z + w2*NextRotation.z;
            Bone.Rotation.w = w1*Rotation.w + w2*NextRotation.w;

            Normalize(&Bone.Rotation);
        }
    }
}

static void DrawBonesSkeleton(Bone_t& Bone)
{
    glPointSize(5.0);
    glTranslatef(Bone.Translation.x, Bone.Translation.y, Bone.Translation.z);
    float RotationMatrix[16];
    FindQuaternionMatrix(RotationMatrix, &Bone.Rotation);
    glMultMatrixf(RotationMatrix);

    if(!strcmp(Bone.Name, "ROOT"))
        glColor3f(1.0, 0.0, 0.0);
    else if(!strcmp(Bone.Name, "HEAD"))
        glColor3f(1.0, 1.0, 0.0);
    else
        glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POINTS); glVertex3f(0, 0, 0); glEnd();

    if(Bone.ChildrenCount == 1){
        DrawBonesSkeleton(*Bone.Children[0]);
    }else if(Bone.ChildrenCount > 1){
        for(unsigned i=0; i<Bone.ChildrenCount; i++){
            glPushMatrix();
            DrawBonesSkeleton(*Bone.Children[i]);
            glPopMatrix();
        }
    }
}

static void DrawSkeleton()
{
    glPushMatrix();
    DrawBonesSkeleton(Skeleton.Bones[0]);
    glPopMatrix();
}

static int DrawScene(float TimeDelta, uint8_t keys[256])
{
    //Handle user interaction
    if(keys['A'])      /*{if(zoom <=-1.0f)  zoom+=0.05f; }*/ zoom+=3*TimeDelta;
    if(keys['S'])      /*{if(zoom >=-10.0f) zoom-=0.05f; }*/ zoom-=3*TimeDelta;
    if(keys[KEY_UP]){    if((Character.Rotation.x-=60*TimeDelta) <=-360) Character.Rotation.x+=360; }
    if(keys[KEY_DOWN]){  if((Character.Rotation.x+=60*TimeDelta) >=360)  Character.Rotation.x-=360; }
    if(keys[KEY_LEFT]){  if((Character.Rotation.y-=60*TimeDelta) <=-360) Character.Rotation.y+=360; }
    if(keys[KEY_RIGHT]){ if((Character.Rotation.y+=60*TimeDelta) >=360)  Character.Rotation.y-=360; }
    if(keys['X']){       if((Character.Rotation.z-=60*TimeDelta) <=-360) Character.Rotation.z+=360; }
    if(keys['Z']){       if((Character.Rotation.z+=60*TimeDelta) >=360)  Character.Rotation.z-=360; }
    if(keys['K']){       Character.Translation.y-=3*TimeDelta; }
    if(keys['I']){       Character.Translation.y+=3*TimeDelta; }
    if(keys['J']){       Character.Translation.x-=3*TimeDelta; }
    if(keys['L']){       Character.Translation.x+=3*TimeDelta; }
    if(keys['Q']){       if(!PressedQ){ PressedQ = 1; ShowSkeleton = !ShowSkeleton; }} else PressedQ = 0;
    if(keys['N']){       AdvanceFrame(Skeleton, Animation, TimeDelta); }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the screen and the depth buffer

    glLoadIdentity();
    glTranslatef(Character.Translation.x, Character.Translation.y, zoom + Character.Translation.z);
    glRotatef(Character.Rotation.x,1.0f,0.0f,0.0f);
    glRotatef(Character.Rotation.y,0.0f,1.0f,0.0f);
    glRotatef(Character.Rotation.z,0.0f,0.0f,1.0f);

    if(ShowMesh){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(1.0, 1.0, 1.0);

        DrawMeshes();
    }

    if(ShowSkeleton){
        glClear(GL_DEPTH_BUFFER_BIT);
        DrawSkeleton();
    }

    return true;
}

static bool Read(const char * Filename, uint8_t ** InData){
    *InData = File::ReadFile(Filename);
    if(*InData != NULL){
        VBFile.set(*InData, File::FileSize);
        return true;
    }

    DisplayFileError(Filename);
    return false;
}

static int Startup()
{
    uint8_t * InData;

    if(!Read("skeleton.skel", &InData))
        return 0;
    ReadSkeleton(Skeleton);
    free(InData);

    for(unsigned i=0; i<MeshCount; i++){
        if(!Read(MeshPaths[i], &InData))
            return 0;
        ReadMesh(Meshes[i]);
        free(InData);
    }

    if(!Read("animation.anim", &InData))
        return 0;
    ReadAnimation(Animation);
    free(InData);

    AdvanceFrame(Skeleton, Animation, 0);
    return 1;
}

extern "C" {
const DemoConfig Demo = {
    "libvitaboy - Renderer",    //Title
    640,480,                    //Width, Height
    Startup,                    //Startup
    NULL,                       //Shutdown
    InitGL,                     //InitGL
    ResizeScene,                //ResizeScene
    DrawScene                   //DrawScene
};
}
