/*
    Niotso - Copyright (C) 2012 Fatbag <X-Fi6@phppoll.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../../EngineInterface.hpp"

static const wchar_t * const StatusStrings[] = {
    L"Extruding Terrain Web",
    L"Adjusting Emotional Weights",
    L"Calibrating Personality Matrix",

    L"Calculating Domestic Coefficients",
    L"Readjusting Career Ladder",
    L"Accessing Money Supply",
    L"Hacking the Social Network",
    L"Tweaking Chaos Control",
    L"Downloading Reticulated Splines"
};
static const char * const images[] = {"eagames.bmp", "maxis.png", "setup.bmp"};

static const char * const sounds[] = {"loadloop.wav"};

LoginScreen::LoginScreen() : Scene(0){
    Screen = Screen_EAGames;
    Time = 0;
    ScrollPos = -1;
    memset(image, 0, IMG_COUNT * sizeof(Image_t *));
    memset(texture, 0, TEX_COUNT * sizeof(GLuint));
    memset(sound, 0, SND_COUNT * sizeof(PlayableSound_t *));

    glMatrixMode(GL_TEXTURE);
    glGenTextures(TEX_COUNT, texture);
    
    FT_Set_Char_Size(Graphics::FontFace, 0, 22*64, 0, 0);
    
    for(int i=TEX_EAGAMES; i<=TEX_SETUP; i++){
        Image_t * Image = File::ReadImageFile(images[i]);
        if(!Image){
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
            }

            char Buffer[1024];
            sprintf(Buffer, Message, images[i]);
            MessageBox(Window::hWnd, Buffer, NULL, MB_OK | MB_ICONERROR);
            EXIT_SCENE();
        }
        
        if(i == TEX_MAXIS){
            Graphics::DrawText(Image, L"Maxis\x2122 is an Electronic Arts\x2122 brand.", 0, 600-146, 800, 146,
                Graphics::ALIGN_CENTER_CENTER, 0, RGB(0xef, 0xe3, 0x8c));
        }
        
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Image->Width, Image->Height, 0, GL_BGR, GL_UNSIGNED_BYTE, Image->Data);
        free(Image->Data);
        free(Image);
    }
    
    image[IMG_COPYRIGHT] = Graphics::StringImage(L"(c) 2002, 2003 Electronic Arts Inc. All rights reserved.",
        0, RGB(0xef, 0xe3, 0x8c));
    if(image[IMG_COPYRIGHT] == NULL){
        EXIT_SCENE();
    }
    glBindTexture(GL_TEXTURE_2D, texture[TEX_COPYRIGHT]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image[IMG_COPYRIGHT]->Width, image[IMG_COPYRIGHT]->Height, 0, GL_BGRA,
        GL_UNSIGNED_BYTE, image[IMG_COPYRIGHT]->Data);
    free(image[IMG_COPYRIGHT]->Data);

    for(int i=0; i<9; i++){
        image[IMG_STATUS+i] = Graphics::StringImage(StatusStrings[i], 0, RGB(0xef, 0xe3, 0x8c));
        if(image[IMG_STATUS+i] == NULL){
            EXIT_SCENE();
        }
        glBindTexture(GL_TEXTURE_2D, texture[TEX_STATUS+i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image[IMG_STATUS+i]->Width, image[IMG_STATUS+i]->Height, 0, GL_BGRA,
            GL_UNSIGNED_BYTE, image[IMG_STATUS+i]->Data);
        free(image[IMG_STATUS+i]->Data);
    }
    
    for(int i=0; i<SND_COUNT; i++){
        Sound_t * Sound = File::ReadSoundFile(sounds[i]);
        if(!Sound){
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
            }

            char Buffer[1024];
            sprintf(Buffer, Message, sounds[i]);
            MessageBox(Window::hWnd, Buffer, NULL, MB_OK | MB_ICONERROR);
            EXIT_SCENE();
        }
        
        sound[i] = Audio::LoadSound(Sound);
        free(Sound);
        if(!sound){
            MessageBox(Window::hWnd, "Sound could not be created", NULL, MB_OK | MB_ICONERROR);
            EXIT_SCENE();
        }
        Audio::PlaySound(sound[i]);
    }
}
    
LoginScreen::~LoginScreen(){
    for(int i=0; i<IMG_COUNT; i++){ if(image[i]) free(image[i]); }
    glDeleteTextures(TEX_COUNT, texture);
    
    for(int i=0; i<SND_COUNT; i++){
        if(sound[i]){
            Audio::DeleteSound(sound[i]);
            free(sound[i]->Data);
            free(sound[i]);
        }
    }
}

int LoginScreen::Run(float TimeDelta){
    Time += TimeDelta;
    if(ScrollPos != 8){
        ScrollPos += TimeDelta*0.75;
        if(ScrollPos > 8) ScrollPos = 8;
    }

    if(Screen != Screen_Setup && Time >= 4.0){
        Screen = (Screen==Screen_EAGames) ? Screen_Maxis : Screen_Setup;
        Time = 0;
    }

    if(System::UserInput.CloseWindow){
        return SCENE_EXIT;
    }
    return SCENE_NEED_REDRAW;
}

void LoginScreen::Render(){
    glMatrixMode(GL_TEXTURE);

    //Background
    glBindTexture(GL_TEXTURE_2D, texture[Screen]);
    glBegin(GL_QUADS);
        glTexCoord2i(0,0); glVertex2i(0,0);
        glTexCoord2i(1,0); glVertex2i(800,0);
        glTexCoord2i(1,1); glVertex2i(800,600);
        glTexCoord2i(0,1); glVertex2i(0,600);
    glEnd();

    if(Screen != Screen_Setup) return;

    glBindTexture(GL_TEXTURE_2D, texture[TEX_COPYRIGHT]);
    glBegin(GL_QUADS);
        glTexCoord2i(0,0); glVertex2i((800-image[IMG_COPYRIGHT]->Width)/2,58);
        glTexCoord2i(1,0); glVertex2i((800-image[IMG_COPYRIGHT]->Width)/2 + image[IMG_COPYRIGHT]->Width,58);
        glTexCoord2i(1,1); glVertex2i((800-image[IMG_COPYRIGHT]->Width)/2 + image[IMG_COPYRIGHT]->Width,image[IMG_COPYRIGHT]->Height + 58);
        glTexCoord2i(0,1); glVertex2i((800-image[IMG_COPYRIGHT]->Width)/2,image[IMG_COPYRIGHT]->Height + 58);
    glEnd();

    for(int i=0; i<9; i++){
        glBindTexture(GL_TEXTURE_2D, texture[TEX_STATUS+i]);
        glBegin(GL_QUADS);
            glTexCoord2i(0,0); glVertex2i(((float)i - ScrollPos)*800 + (800-image[IMG_STATUS+i]->Width)/2,20);
            glTexCoord2i(1,0); glVertex2i(((float)i - ScrollPos)*800 + (800-image[IMG_STATUS+i]->Width)/2 + image[IMG_STATUS+i]->Width,20);
            glTexCoord2i(1,1); glVertex2i(((float)i - ScrollPos)*800 + (800-image[IMG_STATUS+i]->Width)/2 + image[IMG_STATUS+i]->Width,image[IMG_STATUS+i]->Height + 20);
            glTexCoord2i(0,1); glVertex2i(((float)i - ScrollPos)*800 + (800-image[IMG_STATUS+i]->Width)/2,image[IMG_STATUS+i]->Height + 20);
        glEnd();
    }
}