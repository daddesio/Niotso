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

#include <windows.h>		// Header File For Windows
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glext.h>
#include "SOIL.h"
#include "libvitaboy.hpp"

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256] = {0};			// Array Used For The Keyboard Routine
bool	active=TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

bool press = false;

float zoom = -10;
struct CharacterPlacement_t {
    Vertex_t Translation;
    Vertex_t Rotation;
};

CharacterPlacement_t Character = {{0,-3,0}, {0,0,0}};

Skeleton_t Skeleton;

unsigned TextureCount = 3;
GLuint	texture[3];
enum { Texture_Body, Texture_Head, Texture_Hand };
const char* TexturePaths[] = {"body.jpg", "head.jpg", "hand.jpg"};

unsigned MeshCount = 4;
Mesh_t Meshes[4];
enum { Mesh_Body, Mesh_Head, Mesh_LHand, Mesh_RHand };
unsigned Mesh_UseTexture[] = { Texture_Body, Texture_Head, Texture_Hand, Texture_Hand };
const char* MeshActivate[] = {NULL, "HEAD", "L_HAND", "R_HAND"};

Animation_t Animation;

bool ShowMesh = true;
bool ShowSkeleton = true;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

int LoadGLTextures()									// Load Bitmaps And Convert To Textures
{
    for(int i=0; i<3; i++){
        /* load an image file directly as a new OpenGL texture */
        texture[i] = SOIL_load_OGL_texture(TexturePaths[i], SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
        if(texture[i] == 0)
            return false;

        // Typical Texture Generation Using Data From The Bitmap
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
	return true;										// Return Success
}

void ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(void)										// All Setup For OpenGL Goes Here
{
	if (!LoadGLTextures())								// Jump To Texture Loading Routine ( NEW )
	{
		return FALSE;									// If Texture Didn't Load Return FALSE
	}

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
    glEnable(GL_CULL_FACE);
    glEnable(GL_RESCALE_NORMAL);
    glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	return TRUE;										// Initialization Went OK
}

void TransformVertices(Bone_t& Bone){
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
        for(unsigned i=0; i<Mesh.BoneBindings[BoneIndex].FixedVertexCount; i++){
            glTranslatef(Mesh.VertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].x,
                Mesh.VertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].y,
                Mesh.VertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].z);
            glGetFloatv(GL_MODELVIEW_MATRIX, Matrix);
            Mesh.TransformedVertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].x = Matrix[12];
            Mesh.TransformedVertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].y = Matrix[13];
            Mesh.TransformedVertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].z = Matrix[14];
            glTranslatef(-Mesh.VertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].x,
                -Mesh.VertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].y,
                -Mesh.VertexData[Mesh.BoneBindings[BoneIndex].FirstFixedVertex + i].z);
        }
        for(unsigned i=0; i<Mesh.BoneBindings[BoneIndex].BlendedVertexCount; i++){
            glTranslatef(Mesh.VertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].x,
                Mesh.VertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].y,
                Mesh.VertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].z);
            glGetFloatv(GL_MODELVIEW_MATRIX, Matrix);
            Mesh.TransformedVertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].x = Matrix[12];
            Mesh.TransformedVertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].x = Matrix[13];
            Mesh.TransformedVertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].x = Matrix[14];
            glTranslatef(-Mesh.VertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].x,
                -Mesh.VertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].y,
                -Mesh.VertexData[Mesh.FixedVertexCount + Mesh.BoneBindings[BoneIndex].FirstBlendedVertex + i].z);
        }
    }
    
    for(unsigned i=0; i<Bone.ChildrenCount; i++){
        glPushMatrix();
        TransformVertices(*Bone.Children[i]);
        glPopMatrix();
    }
}

void BlendVertices(){
    for(unsigned i=0; i<MeshCount; i++){
        Mesh_t& Mesh = Meshes[i];
        for(unsigned i=0; i<Mesh.BlendedVertexCount; i++){
            Mesh.TransformedVertexData[Mesh.FixedVertexCount + i].x =
                Mesh.BlendData[i].Weight * Mesh.TransformedVertexData[Mesh.FixedVertexCount + i].x +
                (1-Mesh.BlendData[i].Weight) * Mesh.TransformedVertexData[Mesh.BlendData[i].OtherVertex].x;
            Mesh.TransformedVertexData[Mesh.FixedVertexCount + i].y =
                Mesh.BlendData[i].Weight * Mesh.TransformedVertexData[Mesh.FixedVertexCount + i].y +
                (1-Mesh.BlendData[i].Weight) * Mesh.TransformedVertexData[Mesh.BlendData[i].OtherVertex].y;
            Mesh.TransformedVertexData[Mesh.FixedVertexCount + i].z =
                Mesh.BlendData[i].Weight * Mesh.TransformedVertexData[Mesh.FixedVertexCount + i].z +
                (1-Mesh.BlendData[i].Weight) * Mesh.TransformedVertexData[Mesh.BlendData[i].OtherVertex].z;
        }
    }
}

void DrawMeshes(){
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
        glVertexPointer(3, GL_FLOAT, offsetof(Vertex_t, y)-offsetof(Vertex_t, x)-sizeof(float), Meshes[i].TransformedVertexData);
        glTexCoordPointer(2, GL_FLOAT, offsetof(TextureVertex_t, v)-offsetof(TextureVertex_t, u)-sizeof(float), Meshes[i].TransformedTextureData);
        glDrawElements(GL_TRIANGLES, Meshes[i].FaceCount*3, GL_UNSIGNED_INT, Meshes[i].FaceData);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

void AdvanceFrame(Skeleton_t& Skeleton, Animation_t& Animation){
    static unsigned Frame = 0;

    for(unsigned i=0; i<Animation.MotionsCount; i++){
        unsigned BoneIndex = FindBone(Skeleton, Animation.Motions[i].BoneName, Skeleton.BoneCount);
        if(BoneIndex != (unsigned)-1){
            Bone_t& Bone = Skeleton.Bones[BoneIndex];
            
            if(Animation.Motions[i].HasTranslation){
                Translation_t& Translation = Animation.Motions[i].Translations[Frame];
                Bone.Translation.x = Translation.x;
                Bone.Translation.y = Translation.y;
                Bone.Translation.z = Translation.z;
            }
            if(Animation.Motions[i].HasRotation){
                Rotation_t& Rotation = Animation.Motions[i].Rotations[Frame];
                Bone.Rotation.x = Rotation.x;
                Bone.Rotation.y = Rotation.y;
                Bone.Rotation.z = Rotation.z;
                Bone.Rotation.w = Rotation.w;
            }
        }
    }
    
    if(++Frame >= Animation.Motions[0].FrameCount) Frame = 0;
}

void DrawBonesSkeleton(Bone_t& Bone){
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
    
    for(unsigned i=0; i<Bone.ChildrenCount; i++){
        glPushMatrix();
        DrawBonesSkeleton(*Bone.Children[i]);
        glPopMatrix();
    }
}

int DrawGLScene(void)									// Here's Where We Do All The Drawing
{
    if(keys['A'])      /*{if(zoom <=-1.0f)  zoom+=0.05f; }*/ zoom+=0.05f;
    if(keys['S'])      /*{if(zoom >=-10.0f) zoom-=0.05f; }*/ zoom-=0.05f;
    if(keys[VK_UP]){    if((Character.Rotation.x-=1.0f) <=-360) Character.Rotation.x+=360; }
    if(keys[VK_DOWN]){  if((Character.Rotation.x+=1.0f) >=360)  Character.Rotation.x-=360; }
    if(keys[VK_LEFT]){  if((Character.Rotation.y-=1.0f) <=-360) Character.Rotation.y+=360; }
    if(keys[VK_RIGHT]){ if((Character.Rotation.y+=1.0f) >=360)  Character.Rotation.y-=360; }
    if(keys['X']){      if((Character.Rotation.z-=1.0f) <=-360) Character.Rotation.z+=360; }
    if(keys['Z']){      if((Character.Rotation.z+=1.0f) >=360)  Character.Rotation.z-=360; }
    if(keys['K']){      Character.Translation.y-=0.05f; }
    if(keys['I']){      Character.Translation.y+=0.05f; }
    if(keys['J']){      Character.Translation.x-=0.05f; }
    if(keys['L']){      Character.Translation.x+=0.05f; }
    if(keys['N']){      AdvanceFrame(Skeleton, Animation); }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
    
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
        DrawBonesSkeleton(Skeleton.Bones[0]);
    }

	return TRUE;										// Keep Going
}

void KillGLWindow(void)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL CreateGLWindow(const char * title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			// LoWord Can Be WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE,
			// The High-Order Word Specifies The Minimized State Of The Window Being Activated Or Deactivated.
			// A NonZero Value Indicates The Window Is Minimized.
			if ((LOWORD(wParam) != WA_INACTIVE) && !((BOOL)HIWORD(wParam)))
				active=TRUE;						// Program Is Active
			else
				active=FALSE;						// Program Is No Longer Active

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE,		// Instance
					HINSTANCE,		// Previous Instance
					LPSTR,			// Command Line Parameters
					int)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;							// Windowed Mode
	}
    
    HANDLE hFile;
    unsigned FileSize;
    uint8_t *InData;
    DWORD bytestransferred;
    
    hFile = CreateFile("skeleton.skel", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            MessageBox(NULL, "The specified skeleton does not exist.", "Error", MB_OK);
            return 0;
        }
        MessageBox(NULL, "The skeleton could not be opened for reading.", "Error", MB_OK);
        return 0;
    }
    FileSize = GetFileSize(hFile, NULL);
    InData = (uint8_t*) malloc(FileSize);
    if(InData == NULL){
        MessageBox(NULL, "Memory for the skeleton could not be allocated.", "Error", MB_OK);
        return 0;
    }
    if(!ReadFile(hFile, InData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        MessageBox(NULL, "The skeleton could not be read.", "Error", MB_OK);
        return 0;
    }
    CloseHandle(hFile);
    
    VBFile.set(InData, FileSize);
    ReadSkeleton(Skeleton);
    free(InData);

    hFile = CreateFile("body.mesh", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            MessageBox(NULL, "body.mesh does not exist.", "Error", MB_OK);
            return 0;
        }
        MessageBox(NULL, "body.mesh could not be opened for reading.", "Error", MB_OK);
        return 0;
    }
    FileSize = GetFileSize(hFile, NULL);
    InData = (uint8_t*) malloc(FileSize);
    if(InData == NULL){
        MessageBox(NULL, "Memory for body.mesh could not be allocated.", "Error", MB_OK);
        return 0;
    }
    if(!ReadFile(hFile, InData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        MessageBox(NULL, "body.mesh could not be read.", "Error", MB_OK);
        return 0;
    }
    CloseHandle(hFile);
    
    VBFile.set(InData, FileSize);
    ReadMesh(Meshes[0]);
    free(InData);
    
    hFile = CreateFile("head.mesh", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            MessageBox(NULL, "head.mesh does not exist.", "Error", MB_OK);
            return 0;
        }
        MessageBox(NULL, "head.mesh could not be opened for reading.", "Error", MB_OK);
        return 0;
    }
    FileSize = GetFileSize(hFile, NULL);
    InData = (uint8_t*) malloc(FileSize);
    if(InData == NULL){
        MessageBox(NULL, "Memory for head.mesh could not be allocated.", "Error", MB_OK);
        return 0;
    }
    if(!ReadFile(hFile, InData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        MessageBox(NULL, "head.mesh could not be read.", "Error", MB_OK);
        return 0;
    }
    CloseHandle(hFile);
    
    VBFile.set(InData, FileSize);
    ReadMesh(Meshes[1]);
    free(InData);

    hFile = CreateFile("lhand.mesh", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            MessageBox(NULL, "lhand.mesh does not exist.", "Error", MB_OK);
            return 0;
        }
        MessageBox(NULL, "lhand.mesh could not be opened for reading.", "Error", MB_OK);
        return 0;
    }
    FileSize = GetFileSize(hFile, NULL);
    InData = (uint8_t*) malloc(FileSize);
    if(InData == NULL){
        MessageBox(NULL, "Memory for lhand.mesh could not be allocated.", "Error", MB_OK);
        return 0;
    }
    if(!ReadFile(hFile, InData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        MessageBox(NULL, "lhand.mesh could not be read.", "Error", MB_OK);
        return 0;
    }
    CloseHandle(hFile);
    
    VBFile.set(InData, FileSize);
    ReadMesh(Meshes[2]);
    free(InData);

    hFile = CreateFile("rhand.mesh", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            MessageBox(NULL, "rhand.mesh does not exist.", "Error", MB_OK);
            return 0;
        }
        MessageBox(NULL, "rhand.mesh could not be opened for reading.", "Error", MB_OK);
        return 0;
    }
    FileSize = GetFileSize(hFile, NULL);
    InData = (uint8_t*) malloc(FileSize);
    if(InData == NULL){
        MessageBox(NULL, "Memory for rhand.mesh could not be allocated.", "Error", MB_OK);
        return 0;
    }
    if(!ReadFile(hFile, InData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        MessageBox(NULL, "rhand.mesh could not be read.", "Error", MB_OK);
        return 0;
    }
    CloseHandle(hFile);
    
    VBFile.set(InData, FileSize);
    ReadMesh(Meshes[3]);
    free(InData);
    
    hFile = CreateFile("animation.anim", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        if(GetLastError() == ERROR_FILE_NOT_FOUND){
            MessageBox(NULL, "animation.anim does not exist.", "Error", MB_OK);
            return 0;
        }
        MessageBox(NULL, "animation.anim could not be opened for reading.", "Error", MB_OK);
        return 0;
    }
    FileSize = GetFileSize(hFile, NULL);
    InData = (uint8_t*) malloc(FileSize);
    if(InData == NULL){
        MessageBox(NULL, "Memory for animation.anim could not be allocated.", "Error", MB_OK);
        return 0;
    }
    if(!ReadFile(hFile, InData, FileSize, &bytestransferred, NULL) || bytestransferred != FileSize){
        MessageBox(NULL, "animation.anim could not be read.", "Error", MB_OK);
        return 0;
    }
    CloseHandle(hFile);
    
    VBFile.set(InData, FileSize);
    ReadAnimation(Animation);
    free(InData);
    
    AdvanceFrame(Skeleton, Animation);

	// Create Our OpenGL Window
	if (!CreateGLWindow("libvitaboy - Renderer",640,480,16,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done=TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
			}

			if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1]=FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("NeHe's Solid Object Tutorial",640,480,16,fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
