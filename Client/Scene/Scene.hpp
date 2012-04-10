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

#include <GL/gl.h>

#define EXIT_SCENE() do { System::SceneFailed = true; delete this; return; } while(0)
#define SCENE_EXIT        0
#define SCENE_NEED_REDRAW 1
#define SCENE_NO_REDRAW   -1

class Scene {
    const float TickPeriod;
    float RealTimeDelta;
    virtual int Run(float TimeDelta) = 0;

  protected:
    float TimeDelta;
    Scene(float c) : TickPeriod(c), RealTimeDelta(0) {}

  public:
    int RunFor(float TimeDelta) {
        if(TickPeriod == 0){
            return Run(TimeDelta);
        }
        
        bool Redraw = false;
        RealTimeDelta += TimeDelta;
        while(RealTimeDelta >= 0){
            int result = Run(TickPeriod);
            if(result == System::SHUTDOWN)
                return System::SHUTDOWN;
            if(result > 0) Redraw = true;
            RealTimeDelta -= TickPeriod;
        }
        return (Redraw) ? 1 : -1;
    }
    
    virtual void Render() = 0;
    virtual ~Scene() {};
};

enum {
    TEX_EAGAMES,
    TEX_MAXIS,
    TEX_SETUP,
    TEX_COUNT
};

static const char * const images[] = {"eagames.bmp", "maxis.png", "setup.bmp"};

class LoginScreen : public Scene {
    float Time;
    GLuint texture[TEX_COUNT];

  public:
    LoginScreen() : Scene(1.0f/15){
        Time = 0;

        glMatrixMode(GL_TEXTURE);
        glGenTextures(TEX_COUNT, texture);
        
        for(int i=0; i<TEX_COUNT; i++){
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
                FT_Set_Char_Size(Graphics::FontFace, 0, 22*64, 0, 0);
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
    }
    
    ~LoginScreen(){
        glDeleteTextures(TEX_COUNT, texture);
    }

  private:
    int Run(float TimeDelta){
        Time += TimeDelta;

        if(System::UserInput.CloseWindow){
            return SCENE_EXIT;
        }
        return SCENE_NEED_REDRAW;
    }

  public:
    void Render(){
        glMatrixMode(GL_TEXTURE);
        glBindTexture(GL_TEXTURE_2D, texture[(Time>=4) + (Time>=8)]);
        glBegin(GL_QUADS);
            glTexCoord2i(0,0); glVertex2i(0,0);
            glTexCoord2i(1,0); glVertex2i(800,0);
            glTexCoord2i(1,1); glVertex2i(800,600);
            glTexCoord2i(0,1); glVertex2i(0,600);
        glEnd();
    }
};