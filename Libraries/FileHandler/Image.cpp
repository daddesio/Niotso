/*
    FileHandler - General-purpose file handling library for Niotso
    Image.cpp - Copyright (c) 2011-2012 Niotso Project <http://niotso.org/>
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

#include "FileHandler.hpp"
#include <jpeglib.h>
#include <jerror.h>
#include <png.h>
#include <setjmp.h> //Used for libpng
#include "bmp/read_bmp.h"

namespace File {

enum ImageType {
    FIMG_BMP,
    FIMG_JPEG,
    FIMG_PNG,
    FIMG_TGACUR,
    FIMG_COUNT
};

static uint8_t * ReadJPG(Image_t * Image, const uint8_t * InData, size_t FileSize);
static uint8_t * ReadBMP(Image_t * Image, const uint8_t * InData, size_t FileSize);
static uint8_t * ReadPNG(Image_t * Image, const uint8_t * InData, size_t FileSize);
static uint8_t * ReadTGA(Image_t * Image, const uint8_t * InData, size_t FileSize);
static uint8_t * ReadCUR(Image_t * Image, const uint8_t * InData, size_t FileSize);

static uint8_t * ReadTGACUR(Image_t * Image, const uint8_t * InData, size_t FileSize){
    //Microsoft and Truevision, y u no more creative with your file signatures?
    //In many cases we're going to see these bytes, exactly, at the beginning in both formats:
    //00 00 02 00 01 00
    //So screw it. Try parsing the file first as a TGA, then as a CUR.

    uint8_t * Result = ReadTGA(Image, InData, FileSize);
    return Result ? Result : ReadCUR(Image, InData, FileSize);
}

static const uint8_t Signature[] = {
    'B',  //BMP
    0xFF, //JPEG
    0x89, //PNG
    0x00  //TGA or CUR
};
static uint8_t* (* const ImageFunction[])(Image_t*, const uint8_t*, size_t) = {
    ReadBMP,
    ReadJPG,
    ReadPNG,
    ReadTGACUR
};

Image_t * ReadImageFile(const char * Filename){
    uint8_t * InData = File::ReadFile(Filename);
    if(InData == NULL) return NULL;

    if(File::FileSize < 4){
        free(InData);
        File::Error = FERR_INVALIDDATA;
        return NULL;
    }

    Image_t * Image = (Image_t*) malloc(sizeof(Image_t));
    if(Image == NULL){
        free(InData);
        File::Error = FERR_MEMORY;
        return NULL;
    }

    for(int i=0; i<FIMG_COUNT; i++){
        if(InData[0] == Signature[i]){
            uint8_t * OutData = ImageFunction[i](Image, InData, File::FileSize);
            free(InData);
            if(OutData == NULL){
                File::Error = FERR_INVALIDDATA;
                return NULL;
            }
            return Image;
        }
    }

    free(InData);
    File::Error = FERR_UNRECOGNIZED;
    return NULL;
}

static uint8_t * ReadBMP(Image_t * Image, const uint8_t * InData, size_t FileSize){
    bmpheader_t BMPHeader;
    if(!bmp_read_header(&BMPHeader, InData, FileSize)){
        return NULL;
    }

    uint8_t * OutData = (uint8_t*) malloc(BMPHeader.DecompressedSize);
    if(OutData == NULL){
        return NULL;
    }
    if(!bmp_read_data(&BMPHeader, InData, OutData)){
        free(OutData);
        return NULL;
    }

    Image->Width = BMPHeader.biWidth;
    Image->Height = BMPHeader.biHeight;
    Image->Format = FIMG_BGR24;
    Image->Data = OutData;
    return OutData;
}


// libjpeg-turbo v6 doesn't support jpeg_mem_src, so we have to implement it here
static void term_source(j_decompress_ptr){}
static int fill_mem_input_buffer(j_decompress_ptr cinfo){
    ERREXIT(cinfo, JERR_FILE_READ);
    return FALSE;
}
static void skip_input_data(j_decompress_ptr cinfo, long bytes)
{
    jpeg_source_mgr * src = cinfo->src;

	if(bytes > (long) src->bytes_in_buffer){
        ERREXIT(cinfo, JERR_FILE_READ);
        return;
	}
    src->next_input_byte += bytes;
    src->bytes_in_buffer -= bytes;
}
static uint8_t * ReadJPG(Image_t * Image, const uint8_t * InData, size_t FileSize){
    //Initialize
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    if (cinfo.src == NULL)
        cinfo.src = (jpeg_source_mgr *)
            (*cinfo.mem->alloc_small)((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(jpeg_source_mgr));

    jpeg_source_mgr *src = cinfo.src;
    src->init_source = term_source;
    src->fill_input_buffer = fill_mem_input_buffer;
    src->skip_input_data = skip_input_data;
    src->resync_to_restart = jpeg_resync_to_restart;
    src->term_source = term_source;
    src->bytes_in_buffer = FileSize;
    src->next_input_byte = InData;

    if(jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK){
        jpeg_destroy_decompress(&cinfo);
        return NULL;
    }
    cinfo.out_color_space = JCS_EXT_BGR;
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
    for(unsigned i=cinfo.output_height; i; i--){
        //According to the libjpeg documentation,
        //jpeg_read_scanlines can only really read 1 scanline at a time.
        //We need to convert to bottom-up format anyway.
        uint8_t * Location = OutData + (i-1)*row_stride;
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

struct pngdata_t {
    const uint8_t * buffer;
    size_t size;
};
static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length){
    pngdata_t *pngdata = (pngdata_t *) png_get_io_ptr(png_ptr);
    if(length > pngdata->size) png_error(png_ptr, "");
    memcpy(data, pngdata->buffer, length);
    pngdata->buffer += length;
    pngdata->size -= length;
}
static uint8_t * ReadPNG(Image_t * Image, const uint8_t * InData, size_t FileSize){
    pngdata_t pngdata;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr == NULL) return 0;
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL){
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return NULL;
    }
    if(setjmp(png_jmpbuf(png_ptr))){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    pngdata.buffer = InData;
    pngdata.size = FileSize;
    png_set_read_fn(png_ptr, &pngdata, user_read_data);
    png_set_user_limits(png_ptr, 4096, 4096);
    png_read_png(png_ptr, info_ptr,
        PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_STRIP_ALPHA |
        PNG_TRANSFORM_PACKING | PNG_TRANSFORM_GRAY_TO_RGB | PNG_TRANSFORM_BGR, NULL);

    //png_get_IHDR does not work in high-level mode.
    unsigned width = png_get_image_width(png_ptr, info_ptr);
    unsigned height = png_get_image_height(png_ptr, info_ptr);
    uint8_t * OutData = (uint8_t *) malloc(width*height*3);
    if(OutData == NULL){
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    uint8_t **Scanlines = png_get_rows(png_ptr, info_ptr);
    for(unsigned i=0; i<height; i++)
        memcpy(OutData + i*width*3, Scanlines[height-i-1], width*3);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    Image->Width = width;
    Image->Height = height;
    Image->Format = FIMG_BGR24;
    Image->Data = OutData;
    return OutData;
}

static uint8_t * ReadTGA(Image_t * Image, const uint8_t * InData, size_t FileSize){
    return NULL;
}

static uint8_t * ReadCUR(Image_t * Image, const uint8_t * InData, size_t FileSize){
    return NULL;
}

}
