#include <windows.h>
#include <png.h>

extern HDC hDC;

char * buffer;

void user_read_data(png_structp, png_bytep data, png_size_t length){
	memcpy(data, buffer, length);
	buffer += length;
}

HBITMAP ReadPNGIcon(int ID){
    HRSRC resource = FindResource(NULL, MAKEINTRESOURCE(ID), RT_RCDATA);
    buffer = (char *) LockResource(LoadResource(NULL, resource));

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_set_read_fn(png_ptr, NULL, user_read_data);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type;

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);
    else if(color_type == PNG_COLOR_TYPE_PALETTE || color_type == PNG_COLOR_MASK_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
    png_set_bgr(png_ptr);

    png_read_update_info(png_ptr, info_ptr);
    BITMAPINFOHEADER bmi = {
        sizeof(BITMAPINFOHEADER), //biSize
        width,          //biWidth
        height,         //biHeight
        1,              //biPlanes
        32,             //biBitCount
        BI_RGB,         //biCompression
        width*height*4, //biSizeImage
        0,              //biXPelsPerMeter
        0,              //biYPelsPerMeter
        0,              //biClrUsed
        0               //biClrImportant
    };

    unsigned char *buffer;
    png_bytep row_pointers[height];
    HBITMAP hBmp = CreateDIBSection(hDC, (BITMAPINFO*) &bmi, DIB_RGB_COLORS, (void**) &buffer, NULL, 0);
    for(unsigned i=0; i<height; i++)
        row_pointers[i] = buffer + width*4*(height-i-1);
    png_read_image(png_ptr, row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    
    //Convert from BGRA to pre-multiplied BGRA (PBGRA)
    //Description here: http://msdn.microsoft.com/en-us/library/ee719797%28VS.85%29.aspx
    for(unsigned i=0; i<width*height*4; i+=4){
        unsigned alpha = buffer[i+3];
        int j;
        for(j=0; j<3; j++)
            buffer[i+j] = (char)((unsigned) buffer[i+j]*alpha/255);
    }
    
    return hBmp;
}