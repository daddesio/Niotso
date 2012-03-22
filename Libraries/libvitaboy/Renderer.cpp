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
    n: Animate the character
    F11: Enter/leave fullscreen
*/

#include <math.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glext.h>
#include <FileHandler.hpp>
#include "libvitaboy.hpp"

HDC       hDC=NULL;
HGLRC     hRC=NULL;
HWND      hWnd=NULL;
HINSTANCE hInstance;

bool keys[256] = {0};
bool active=true;
bool fullscreen=false;

float zoom = -10;
struct BasicVertex_t {
    float x, y, z;
};
struct CharacterPlacement_t {
    BasicVertex_t Translation;
    BasicVertex_t Rotation;
};
CharacterPlacement_t Character = {{0,-3,0}, {0,0,0}};

Skeleton_t Skeleton;

const unsigned TextureCount = 3;
unsigned texture[3];
enum { Texture_Body, Texture_Head, Texture_Hand };
const char* const TexturePaths[] = {"body.jpg", "head.jpg", "hand.jpg"};

const unsigned MeshCount = 4;
Mesh_t Meshes[4];
enum { Mesh_Body, Mesh_Head, Mesh_LHand, Mesh_RHand };
const char* const MeshPaths[]    = {"body.mesh", "head.mesh", "lhand.mesh", "rhand.mesh" };
const unsigned Mesh_UseTexture[] = { Texture_Body, Texture_Head, Texture_Hand, Texture_Hand };
const char* const MeshActivate[] = {NULL, "HEAD", "L_HAND", "R_HAND"};

Animation_t Animation;
float AnimationTime = 0;

bool ShowMesh = true;
bool ShowSkeleton = true;

LARGE_INTEGER ClockFreq, PreviousTime;
float FramePeriod;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void DisplayFileError(const char * Filename){
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
    MessageBox(hWnd, Buffer, NULL, MB_OK | MB_ICONERROR);
}

bool LoadTextures()
{
    glGenTextures(3, texture);
    for(int i=0; i<3; i++){
        Image_t * Image = File::ReadImageFile(TexturePaths[i]);
        if(!Image){
            DisplayFileError(TexturePaths[i]);
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->Width, Image->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Image->Data);
        free(Image->Data);
        free(Image);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    return true;
}

void ResizeScene(GLsizei width, GLsizei height)
{
    if(height==0) height++;

    glViewport(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

bool InitGL()
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
    return true;
}

void TransformVertices(Bone_t& Bone)
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

void BlendVertices()
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

void DrawMeshes()
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

void AdvanceFrame(Skeleton_t& Skeleton, Animation_t& Animation, float TimeDelta)
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

            //Use Slerp to interpolate
            float w1, w2 = 1;
            float cosTheta = DotProduct(&Rotation, &NextRotation);
            if(cosTheta < 0){
                cosTheta *= -1;
                w2 *= -1;
            }
            float theta    = (float) acos(cosTheta);
            float sinTheta = (float) sin(theta);

            if(sinTheta > 0.001f){
                w1 =  (float) sin((1.0f-FractionShown)*theta)/sinTheta;
                w2 *= (float) sin(FractionShown       *theta)/sinTheta;
            } else {
                w1 = 1.0f - FractionShown;
                w2 = FractionShown;
            }

            Bone.Rotation.x = w1*Rotation.x + w2*NextRotation.x;
            Bone.Rotation.y = w1*Rotation.y + w2*NextRotation.y;
            Bone.Rotation.z = w1*Rotation.z + w2*NextRotation.z;
            Bone.Rotation.w = w1*Rotation.w + w2*NextRotation.w;
        }
    }
}

void DrawBonesSkeleton(Bone_t& Bone)
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

void DrawSkeleton()
{
    glPushMatrix();
    DrawBonesSkeleton(Skeleton.Bones[0]);
    glPopMatrix();
}

int DrawGLScene()
{
    //Obtain the current time
    LARGE_INTEGER CurrentTime;
    QueryPerformanceCounter(&CurrentTime);
    float TimeDelta = (float)(CurrentTime.QuadPart-PreviousTime.QuadPart)/ClockFreq.QuadPart;
    if(TimeDelta < 0) TimeDelta = 0; //Safe-guard in case of system delay
    PreviousTime = CurrentTime;

    //Handle user interaction
    if(keys['A'])      /*{if(zoom <=-1.0f)  zoom+=0.05f; }*/ zoom+=3*TimeDelta;
    if(keys['S'])      /*{if(zoom >=-10.0f) zoom-=0.05f; }*/ zoom-=3*TimeDelta;
    if(keys[VK_UP]){    if((Character.Rotation.x-=60*TimeDelta) <=-360) Character.Rotation.x+=360; }
    if(keys[VK_DOWN]){  if((Character.Rotation.x+=60*TimeDelta) >=360)  Character.Rotation.x-=360; }
    if(keys[VK_LEFT]){  if((Character.Rotation.y-=60*TimeDelta) <=-360) Character.Rotation.y+=360; }
    if(keys[VK_RIGHT]){ if((Character.Rotation.y+=60*TimeDelta) >=360)  Character.Rotation.y-=360; }
    if(keys['X']){      if((Character.Rotation.z-=60*TimeDelta) <=-360) Character.Rotation.z+=360; }
    if(keys['Z']){      if((Character.Rotation.z+=60*TimeDelta) >=360)  Character.Rotation.z-=360; }
    if(keys['K']){      Character.Translation.y-=3*TimeDelta; }
    if(keys['I']){      Character.Translation.y+=3*TimeDelta; }
    if(keys['J']){      Character.Translation.x-=3*TimeDelta; }
    if(keys['L']){      Character.Translation.x+=3*TimeDelta; }
    if(keys['N']){      AdvanceFrame(Skeleton, Animation, TimeDelta); }

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

void KillGLWindow()
{
    if(fullscreen){
        ChangeDisplaySettings(NULL, 0); //Reset to the desktop resolution
        ShowCursor(true);
    }

    if(hRC){
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hRC);
        hRC = NULL;
    }

    if(hDC){
        ReleaseDC(hWnd,hDC);
        hDC = NULL;
    }

    if(hWnd){
        DestroyWindow(hWnd);
        hWnd = NULL;
    }

    UnregisterClass("OpenGL", hInstance);
    hInstance = NULL;
}

BOOL CreateGLWindow(const char * title, int width, int height, int bits, bool fullscreenflag)
{
    fullscreen = fullscreenflag;
    hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {
        CS_HREDRAW | CS_VREDRAW | CS_OWNDC, //style
        (WNDPROC) WndProc,   //lpfnWndProc
        0,                   //cbClsExtra
        0,                   //cbWndExtra
        hInstance,           //hInstance
        (HICON) LoadImage(NULL, IDI_WINLOGO, IMAGE_ICON, 0, 0, LR_SHARED), //hIcon
        (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE), //hCursor
        NULL,                //hbrBackground
        NULL,                //lpszMenuName
        "OpenGL"             //lpszClassName
    };

    if(!RegisterClass(&wc)){
        MessageBox(NULL, "Failed to registrer the window class.", NULL, MB_OK | MB_ICONERROR);
        return false;
    }

    DWORD dwStyle, dwExStyle;

    if(fullscreen){
        DEVMODE dmScreenSettings;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings);
        width  = dmScreenSettings.dmPelsWidth;
        height = dmScreenSettings.dmPelsHeight;
        bits   = dmScreenSettings.dmBitsPerPel;

        dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
        dwStyle = WS_POPUP;
        ShowCursor(false);
    }else{
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_OVERLAPPEDWINDOW;
    }

    RECT WindowRect;
    WindowRect.left   = 0;
    WindowRect.right  = width;
    WindowRect.top    = 0;
    WindowRect.bottom = height;
    AdjustWindowRectEx(&WindowRect, dwStyle, false, dwExStyle);

    // Create The Window
    if(!(hWnd = CreateWindowEx(dwExStyle, "OpenGL",
        title,
        dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0,
        WindowRect.right-WindowRect.left,
        WindowRect.bottom-WindowRect.top,
        NULL, NULL, hInstance, NULL))
    ){
        KillGLWindow();
        MessageBox(NULL, "Window creation error.", NULL, MB_OK | MB_ICONERROR);
        return false;
    }

    const PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1, //Size and version
        PFD_DRAW_TO_WINDOW |              //dwFlags
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                    //iPixelType
        bits,                             //cColorBits
        0, 0, 0, 0, 0, 0, 0, 0,           //R,G,B,A bits
        0, 0, 0, 0, 0,                    //Accumulation buffer bits
        16,                               //cDepthBits
        0,                                //cStencilBits
        0,                                //cAuxBuffers
        PFD_MAIN_PLANE,                   //iLayerType
        0,                                //Reserved
        0, 0, 0                           //Masks
    };

    hDC = GetDC(hWnd);
    if(!hDC){
        KillGLWindow();
        MessageBox(NULL, "Failed to create an OpenGL device context.", NULL, MB_OK | MB_ICONERROR);
        return false;
    }

    int PixelFormat = ChoosePixelFormat(hDC, &pfd);
    if(!PixelFormat){
        KillGLWindow();
        MessageBox(NULL, "Can't find a suitable PixelFormat.", NULL, MB_OK | MB_ICONERROR);
        return false;
    }

    if(!SetPixelFormat(hDC, PixelFormat, &pfd)){
        KillGLWindow();
        MessageBox(NULL, "Can't set the PixelFormat.", NULL, MB_OK | MB_ICONERROR);
        return false;
    }

    hRC = wglCreateContext(hDC);
    if(!hRC){
        KillGLWindow();
        MessageBox(NULL, "Failed to create an OpenGL rendering context.", NULL, MB_OK | MB_ICONERROR);
        return false;
    }

    if(!wglMakeCurrent(hDC, hRC)){
        KillGLWindow();
        MessageBox(NULL, "Failed to activate the OpenGL device context.", NULL, MB_OK | MB_ICONERROR);
        return false;
    }

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    ResizeScene(width, height);

    if(!InitGL()){
        KillGLWindow();
        MessageBox(NULL, "Initialization failed.", NULL, MB_OK | MB_ICONERROR);
        return false;
    }

    BOOL (WINAPI *wglSwapIntervalEXT)(int) = (BOOL (WINAPI *)(int)) wglGetProcAddress("wglSwapIntervalEXT");
    if(wglSwapIntervalEXT) wglSwapIntervalEXT(1);
    int (WINAPI *wglGetSwapIntervalEXT)(void) = (int (WINAPI *)(void)) wglGetProcAddress("wglGetSwapIntervalEXT");
    if(wglGetSwapIntervalEXT) wglGetSwapIntervalEXT(); //Seems necessary on some cards

    QueryPerformanceFrequency(&ClockFreq);
    QueryPerformanceCounter(&PreviousTime);

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_KEYDOWN: {
        if(wParam == VK_ESCAPE){
            PostQuitMessage(0);
        }else if(wParam == VK_F11 && !keys[VK_F11]){
            KillGLWindow();
            fullscreen = !fullscreen;
            if(!CreateGLWindow("libvitaboy - Renderer",640,480,24,fullscreen)){
                PostQuitMessage(0);
            }
        }

        keys[wParam] = true;
    } return 0;

    case WM_KEYUP: {
        keys[wParam] = false;
    } return 0;

    case WM_ACTIVATE: {
        // LoWord Can Be WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE,
        // The High-Order Word Specifies The Minimized State Of The Window Being Activated Or Deactivated.
        // A NonZero Value Indicates The Window Is Minimized.
        if ((LOWORD(wParam) != WA_INACTIVE) && !((BOOL)HIWORD(wParam)))
            active = true;
        else
            active = false;
    } return 0;

    case WM_SIZE: {
        ResizeScene(LOWORD(lParam),HIWORD(lParam));
    } return 0;

    case WM_SYSCOMMAND: {
        switch (wParam) {
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
            return 0;
        }
    } break;

    case WM_DEVMODECHANGE: {
        DEVMODE dm;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
        FramePeriod = 1.0f/dm.dmDisplayFrequency;
    } return 0;

    case WM_CLOSE: {
        PostQuitMessage(0);
    } return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool Read(const char * Filename, uint8_t ** InData){
    *InData = File::ReadFile(Filename);
    if(*InData != NULL){
        VBFile.set(*InData, File::FileSize);
        return true;
    }

    DisplayFileError(Filename);
    return false;
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
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

    if(!CreateGLWindow("libvitaboy - Renderer",640,480,16,fullscreen)){
        return 0;
    }

    DEVMODE dm;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    FramePeriod = 1.0f/dm.dmDisplayFrequency;

    bool quit = false;
    MSG msg;
    while(true){
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT)
                quit = true;
        }
        if(quit) break;

        DrawGLScene();
        SwapBuffers(hDC);
        LARGE_INTEGER RenderTime;
        QueryPerformanceCounter(&RenderTime);
        float SleepDuration = (FramePeriod - (float)(RenderTime.QuadPart-PreviousTime.QuadPart)/ClockFreq.QuadPart) * 1000;
        if(SleepDuration > 1) Sleep((unsigned) SleepDuration);
    }

    //Shutdown
    KillGLWindow();
    return (msg.wParam);
}
