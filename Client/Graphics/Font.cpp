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

#include "../EngineInterface.hpp"

namespace Graphics {

FT_Library FreeTypeLibrary;
FT_Face    FontFace;

static void FindStringSize(const wchar_t * String, unsigned * width, unsigned * height, int font){
    int x = 0, y = 0;
    int lowestx = 0, lowesty = 0, highestx = 0, highesty = 0;
    
    for(wchar_t letter=*String; letter!='\0'; letter=*(++String)){
        int error = FT_Load_Char(FontFace, letter, FT_LOAD_RENDER);
        if(error) continue;
        
        if(x < lowestx) lowestx = x;
        if(y < lowesty) lowesty = y;
        
        int newx = x + FontFace->glyph->bitmap.width + FontFace->glyph->bitmap_left;
        int newy = y + FontFace->glyph->bitmap_top;
        if(newx > highestx) highestx = newx;
        if(newy > highesty) highesty = newy;
        
        x += FontFace->glyph->advance.x >> 6;
        y += FontFace->glyph->advance.y >> 6;
    }
    
    *width = highestx-lowestx, *height = highesty-lowesty;
}

void DrawText(Image_t * Image, const wchar_t * String, int x, int y, unsigned width, unsigned height,
    TextAlignment Alignment, int font, COLORREF Color){
    //x, y, width, height form the bounding rectangle into which the text should be drawn.
    //(x,y) defines the offset to the top-left of the rectangle from the top left of the background.
    //The destination image must be stored in bottom-up order.
    
    if(x >= (signed)Image->Width || y >= (signed)Image->Height) return;
    
    int Blue = GetBValue(Color), Green = GetGValue(Color), Red = GetRValue(Color);

    //(stringx,stringy) will refer to the top-left of the string in bottom-up coordinates
    int stringx, stringy;
    unsigned StringWidth, StringHeight;
    FindStringSize(String, &StringWidth, &StringHeight, font);
    
    //Horizontal alignment
    if(Alignment < 2)      stringx = x;                           //Left
    else if(Alignment < 4) stringx = x+(width-StringWidth+1)/2;   //Middle
    else                   stringx = x+width-StringWidth;         //Right
    //Vertical alignment
    if(!(Alignment&1))     stringy = y;                           //Top
    else                   stringy = y+(height-StringHeight+1)/2; //Middle
    stringy = Image->Height-stringy-StringHeight;
    
    //Now that we've done the alignment, we can crop the bounding box within the limits of the background image
    if(x < 0){ Image->Width += x; x = 0; }
    if(y < 0){ Image->Height += y; y = 0; }
    if(width > Image->Width) width = Image->Width;
    if(height > Image->Height) height = Image->Height;
    
    for(wchar_t letter=*String; letter!='\0'; letter=*(++String)){
        int error = FT_Load_Char(FontFace, letter, FT_LOAD_RENDER);
        if(error) continue;
        
        int cWidth = FontFace->glyph->bitmap.width, cHeight = FontFace->glyph->bitmap.rows;
        if(cWidth && cHeight){
            uint8_t * cRender; /* Convert to Bottom-up */
            uint8_t * OriginalRender = FontFace->glyph->bitmap.buffer;
            if(FontFace->glyph->bitmap.pitch > 0){
                cRender = (uint8_t *) malloc(cWidth * cHeight);
                for(int i=0; i<cHeight; i++)
                    memcpy(cRender + i*cWidth, OriginalRender + (cHeight-i-1)*cWidth, cWidth);
            }else cRender = OriginalRender;

            stringx += FontFace->glyph->bitmap_left;
            stringy += FontFace->glyph->bitmap_top-cHeight;
            for(int i=max(-stringy, 0); i<cHeight && (unsigned)stringy+i < height; i++){
                for(int j=max(-stringx, 0); j<cWidth && (unsigned)stringx+j < width; j++){
                    int value = cRender[i*cWidth + j];
                    uint8_t *ptr = Image->Data + 3*((stringy+i)*width + (stringx+j));

                    int originalcolor;
                    originalcolor = *ptr;
                    *ptr++ = (uint8_t) (originalcolor + (int)((Blue-originalcolor)*2*value+255)/510);
                    originalcolor = *ptr;
                    *ptr++ = (uint8_t) (originalcolor + (int)((Green-originalcolor)*2*value+255)/510);
                    originalcolor = *ptr;
                    *ptr++ = (uint8_t) (originalcolor + (int)((Red-originalcolor)*2*value+255)/510);
                }
            }
            stringx -= FontFace->glyph->bitmap_left;
            stringy -= FontFace->glyph->bitmap_top-cHeight;
            
            if(FontFace->glyph->bitmap.pitch > 0) free(cRender);
        }
        stringx += FontFace->glyph->advance.x >> 6;
        stringy += FontFace->glyph->advance.y >> 6;
    }
}

}