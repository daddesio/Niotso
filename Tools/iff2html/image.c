/*
    iff2html - iff web page description generator
    image.c - Copyright (c) 2012 Niotso Project <http://niotso.org/>
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

#include <stdio.h>
#include <iff/iff.h>
#include <bmp/read_bmp.h>
#include <png.h>
#include <setjmp.h> /* Used for libpng */
#include "opngreduc.h"

int WritePNG(const char * OutName, const IFFChunk * ChunkData, int ZBuffer,
    const IFFSprite * Sprite, size_t * Width, size_t * Height){
    FILE * hFile;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep * row_pointers;
    unsigned i;

    struct {
        size_t Width;
        size_t Height;
        uint8_t * Data;
    } Image;

    /* We must swap from BGR to RGB; this cannot be done with libpng when you use
    ** opng_reduce_image due to the state that it leaves png_ptr in */

    if(ChunkData){
        /* BMP_ or FBMP chunk */
        bmpheader_t BMPHeader;

        if(!bmp_read_header(&BMPHeader, ChunkData->Data, ChunkData->Size))
            return 0;

        Image.Data = malloc(BMPHeader.DecompressedSize);
        if(Image.Data == NULL)
            return 0;
        if(!bmp_read_data(&BMPHeader, ChunkData->Data, Image.Data)){
            free(Image.Data);
            return 0;
        }

        Image.Width = BMPHeader.biWidth;
        Image.Height = BMPHeader.biHeight;

        for(i=0; i<Image.Width*Image.Height; i++){
            uint8_t temp = Image.Data[i*3 + 0];
            Image.Data[i*3 + 0] = Image.Data[i*3 + 2];
            Image.Data[i*3 + 2] = temp;
        }
    }else{
        /* SPR# or SPR2 sprite */
        Image.Width = Sprite->Width;
        Image.Height = Sprite->Height;
        Image.Data = (!ZBuffer) ? Sprite->BGRA32Data : Sprite->ZBuffer;

        if(!ZBuffer){
            for(i=0; i<Image.Width*Image.Height; i++){
                uint8_t temp = Image.Data[i*4 + 0];
                Image.Data[i*4 + 0] = Image.Data[i*4 + 2];
                Image.Data[i*4 + 2] = temp;
            }
        }
    }

    row_pointers = malloc(Image.Height * sizeof(png_bytep));
    if(row_pointers == NULL){
        if(ChunkData) free(Image.Data);
        return 0;
    }
    for(i=0; i<Image.Height; i++)
        row_pointers[i] = Image.Data + Image.Width*((ChunkData) ? 3*(Image.Height-i-1) : ((!ZBuffer)?4:1)*i);

    /****
    ** PNG handling
    */

    /* Initialization */
    hFile = fopen(OutName, "wb");
    if(hFile == NULL){
        free(row_pointers);
        if(ChunkData) free(Image.Data);
        return 0;
    }
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr == NULL){
        fclose(hFile);
        free(row_pointers);
        if(ChunkData) free(Image.Data);
        return 0;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL){
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(hFile);
        free(row_pointers);
        if(ChunkData) free(Image.Data);
        return 0;
    }
    if(setjmp(png_jmpbuf(png_ptr))){
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(hFile);
        free(row_pointers);
        if(ChunkData) free(Image.Data);
        return 0;
    }

    png_init_io(png_ptr, hFile);

    png_set_filter(png_ptr, 0, PNG_ALL_FILTERS);
    png_set_compression_level(png_ptr, 9);
    png_set_compression_mem_level(png_ptr, 9);
    png_set_compression_window_bits(png_ptr, 15);
    png_set_compression_buffer_size(png_ptr, 32768);

    png_set_IHDR(png_ptr, info_ptr, Image.Width, Image.Height, 8,
        ChunkData ? PNG_COLOR_TYPE_RGB : (!ZBuffer) ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_GRAY,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_set_rows(png_ptr, info_ptr, row_pointers);
    opng_reduce_image(png_ptr, info_ptr, OPNG_REDUCE_ALL);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(hFile);
    free(row_pointers);

    if(ChunkData){
        free(Image.Data);
        *Width = Image.Width;
        *Height = Image.Height;
    }
    return 1;
}