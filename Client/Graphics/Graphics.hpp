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

#include <gl/gl.h>
#include <gl/glext.h>

//Graphics/Startup.cpp
namespace Graphics {
    int Initialize();
    void Shutdown();
    extern HDC hDC;
    extern HGLRC hRC;
    
    int InitGL();
    void ResizeViewport(unsigned width, unsigned height);
    
    enum TextAlignment {
        ALIGN_LEFT_TOP,
        ALIGN_LEFT_CENTER,
        ALIGN_CENTER_TOP,
        ALIGN_CENTER_CENTER,
        ALIGN_RIGHT_TOP,
        ALIGN_RIGHT_CENTER
    };
    
    //Font.cpp
    extern FT_Library FreeTypeLibrary;
    extern FT_Face    FontFace;
    void DrawText(Image_t * Image, const wchar_t * String, int x, int y, unsigned width, unsigned height,
        TextAlignment Alignment, int font, COLORREF Color);
    Image_t * StringImage(const wchar_t * String, int font, COLORREF Color);
}