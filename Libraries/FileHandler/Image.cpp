/*
    FileHandler - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

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
#include <memory.h>
#include "libjpeg-turbo/jpeglib.h"
#define NOWINDOWS
#include "FileHandler.hpp"

namespace File {

Image_t * ReadImageFile(const char * Filename){
    uint8_t * InData = File::ReadFile(Filename);
    if(InData == NULL) return NULL;

    Image_t * Image = (Image_t*) malloc(sizeof(Image_t));
    if(Image == NULL){
        free(InData);
        return NULL;
    }

    uint8_t * OutData = ReadJPG(Image, InData, File::FileSize);

    free(InData);
    if(OutData != NULL){
        return Image;
    }

    File::Error = FERR_UNRECOGNIZED;
    free(Image);
    return NULL;
}

uint8_t * ReadJPG(Image_t * Image, const uint8_t * InData, size_t FileSize){
    //Initialize
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, (unsigned char*) InData, FileSize);
    if(jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK){
        jpeg_destroy_decompress(&cinfo);
        return NULL;
    }
    if(!jpeg_start_decompress(&cinfo)){
        jpeg_destroy_decompress(&cinfo);
        return NULL;
    }

    //Read
    unsigned row_stride = cinfo.output_width * cinfo.output_components;
    uint8_t * OutData = (uint8_t*) malloc(cinfo.output_width * cinfo.output_height * cinfo.output_components);
    if(OutData == NULL){
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        return NULL;
    }
    for(unsigned i=0; cinfo.output_scanline < cinfo.output_height; i++){
        //According to the libjpeg documentation,
        //jpeg_read_scanlines can only really read 1 scanline at a time.
        uint8_t * Location = OutData + i*row_stride;
        if(!jpeg_read_scanlines(&cinfo, &Location, 1)){
            free(OutData);
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
            return NULL;
        }
    }

    //Close up
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    if(Image){
        Image->Width = cinfo.output_width;
        Image->Height = cinfo.output_height;
        Image->Format = FIMG_BGR24;
        Image->Data = OutData;
    }
    return OutData;
}

}