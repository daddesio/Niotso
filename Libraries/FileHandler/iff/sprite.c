#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <png.h>
#include "iff.h"


int sprite_frame_set_texel(IFFSpriteFrame *frame, uint32_t column, uint32_t row, IFFSpriteColor color);
int sprite_frame_export_as_targa(IFFSpriteFrame *frame, const char *filename);
int sprite_frame_export_as_png(IFFSpriteFrame *frame, const char *filename);

int iff_parse_sprite(IFFChunk * ChunkInfo, const uint8_t * Buffer, IFFFile *SourceFile)
{
    uint32_t frameCount = 0;
    uint32_t paletteID = 0;
	
	uint32_t *offsets;
    
    IFFChunk *PaletteMap;
    IFFPMap *PMap;
    
    IFFSprite *Sprite;
    IFFSpriteFrame *Frame;
    
    uint32_t i = 0;
    uint32_t l = 0;
    
    uint32_t row = 0;
    uint32_t column = 0;
    uint8_t quit = 0;
    uint32_t numCodesTillNewline = 0;
    
    uint16_t encrypted = 0;
    uint16_t encryptedb = 0;
    uint16_t decrypted1 = 0;
    uint16_t decrypted2 = 0;
    uint16_t decryptedb3 = 0;
    uint16_t decryptedb4 = 0;
    
    uint32_t bytesRead = 0;
    uint8_t Z;
    IFFSpriteColor clr;
    
    uint32_t rowHeader[2];
    uint32_t rowHeader2[2];
    
    uint8_t b;
    uint8_t b1;
    
    
#ifdef IFF2HTML
    char *outputPath;
    char *dummyName;
    char *folderName;
#endif
    
    uint32_t version = read_uint32le(Buffer);
	Buffer += 4;

    if (version == 1001)
    {
        paletteID = read_uint32le(Buffer);
		Buffer += 4;
        frameCount = read_uint32le(Buffer);
		Buffer += 4;
    }
    else
    {
        frameCount = read_uint32le(Buffer);
		Buffer += 4;
        paletteID = read_uint32le(Buffer);
		Buffer += 4;
    }
    
    /* Try to load the appropriate palette */
    PaletteMap = iff_find_first_chunk(SourceFile, "PALT", paletteID);
    /* If that didn't work, try loading any palette from the IFF file */
    /* Some sprites in IFFs containing only one PALT don't bother to specify a correct PALT ID */
    if (PaletteMap == NULL)
    {
        PaletteMap = iff_find_first_chunk(SourceFile, "PALT", 0);
        /* If there is no existing palette data, there can be no coherent image. */
        if (PaletteMap == NULL)
            return 0;
    }
    PMap = (IFFPMap *)PaletteMap->FormattedData;

    offsets = (uint32_t *)malloc(sizeof(uint32_t) * frameCount);
	memset(offsets, 0, sizeof(uint32_t) * frameCount);

    if (version == 1000)
    {
        for (i = 0; i < frameCount; i++)
        {
            offsets[i] = read_uint32le(Buffer);
            Buffer += 4;
        }
    }
    
    Sprite = (IFFSprite *)malloc(sizeof(IFFSprite));
    Sprite->FrameCount = frameCount;
    Sprite->Frames = (IFFSpriteFrame **)malloc(sizeof (IFFSpriteFrame *) * frameCount);
    
#ifdef IFF2HTML
    Sprite->Version = version;
#endif
    
    for (l = 0; l < frameCount; l++)
    {
        Frame = (IFFSpriteFrame *)malloc(sizeof(IFFSpriteFrame));
    
        /* Version 1000 specifies offsets for each frame of image data */
        if (version == 1000)
            Buffer = ChunkInfo->Data + offsets[l];

        /* There are two "+=" statements here for clarity. That is optimized away by a decent compiler */
        if (version == 1001)
        {
            Buffer += 4;                        /* Version */
            Buffer += 4;                        /* Size    */
        }
        if (version != 1000 && version != 1001) /* for SPR# resources */
        {
            Frame->YLocation = read_uint16le(Buffer);
            Buffer += 2;
            Frame->YLocation = read_uint16le(Buffer);
            Buffer += 2;
            Frame->Height = read_uint16le(Buffer);
            Buffer += 2;
            Frame->Width = read_uint16le(Buffer);
            Buffer += 2;
        }
        else
        {
            Frame->Width = read_uint16le(Buffer);
            Buffer += 2;
            Frame->Height = read_uint16le(Buffer);
            Buffer += 2;
        }
        
        Frame->Flag = read_uint16le(Buffer);
        Buffer += 2;

        if (version == 1000 || version == 1001)
        {
            Buffer += 2;
            Frame->PaletteID = read_uint16le(Buffer); /* This is unused or the same as the master PALT ID */
            Buffer += 2;
            Frame->TransparentPixel = PMap->Colors[read_uint16le(Buffer)];
            Buffer += 2;
            Frame->YLocation = read_uint16le(Buffer);
            Buffer += 2;
            Frame->XLocation = read_uint16le(Buffer);
            Buffer += 2;
        }

        /* Now that we know the frame size, allocate the buffer to hold its texels */
        Frame->Texels = (IFFSpriteColor *)malloc(sizeof(IFFSpriteColor) * Frame->Width * Frame->Height);

        if (version == 1000 || version == 1001)
        {
            while (quit == 0)
            {
                encrypted = read_uint16le(Buffer);
                encryptedb = 0;
                decrypted1 = encrypted>>13;
                decrypted2 = encrypted&0x1FFF;
                
                
                
                decryptedb3 = 0;
                decryptedb4 = 0;
                Buffer += 2;
                
                

                
                switch (decrypted1)
                {
                    case 0:
                        column = 0;
                        numCodesTillNewline = decrypted2;

                        bytesRead = 0;
                        for (bytesRead = 0; bytesRead < numCodesTillNewline - 2; bytesRead += 2)
                        {
                            encryptedb = read_uint16le(Buffer);
                            Buffer += 2;
                            
                            decryptedb3 = encryptedb>>13;
                            decryptedb4 = encryptedb&0x1FFF;

                            switch (decryptedb3)
                            {
                                case 1:
                                    
                                    for (i = 0; i < decryptedb4; i++)
                                    {
                                        Z = *Buffer++;      /* TODO: Use the z-buffer */
                                        b = *Buffer++;
                                        sprite_frame_set_texel(Frame, column++, row, PMap->Colors[b]);
                                        /*c = PMap->Colors[b];       This variable is not used */
                                        bytesRead += 2;
                                    }
                                    break;
                                case 2:
                                    i = 0;
                                    for (i = 0; i < decryptedb4; i++)
                                    {
                                        Z = *Buffer++;      /* TODO: Use the z-buffer */
                                        b = *Buffer++;
                                        
                                        clr = PMap->Colors[b];
                                        clr.A = *Buffer++;
                                        sprite_frame_set_texel(Frame, column++, row, clr);
                                        bytesRead += 3;
                                    }
                                    if ((Buffer - ChunkInfo->Data) % 2 == 1) 
                                    { 
                                        Buffer++;
                                        bytesRead++;
                                    }
                                    break;
                                case 3:
                                    column += decryptedb4;
                                    break;
                                case 6:
                                    i = 0;
                                    for (i = 0; i < decryptedb4; i++)
                                    {
                                        b = *Buffer++;
                                        sprite_frame_set_texel(Frame, column++, row, PMap->Colors[b]);
                                        bytesRead++;
                                    }

                                    if ((Buffer - ChunkInfo->Data) % 2 == 1) 
                                    { 
                                        Buffer++;
                                        bytesRead++;
                                    }

                                    break;
                                default:
                                    break;
                            }
                        }
                        row++;
                        
                    break;
                    
                    case 4:
                        i = 0;
                        for (i = 0; i < decrypted2; i++)
                        {
                            row++;
                            column = 0;
                        }
                        break;
                    case 5:
                        quit = 1;
                        break;
                    default:
                        /* Error reading code */
                        return 0;
                }
                
                if ((uint32_t)(Buffer - ChunkInfo->Data) == ChunkInfo->Size)
                    break;
            }
        }
        else
        {
            while (quit == 0)
            {
                b = 0;
                b1 = 0;
                /*b2 = 0;     This variable is unused */
                rowHeader[0] = *Buffer++;
                rowHeader[1] = *Buffer++;
                switch (rowHeader[0])
                {
                    case 4:
                        column = 0;
                        numCodesTillNewline = rowHeader[1];

                        bytesRead = 0;
                        for (bytesRead = 0; bytesRead < numCodesTillNewline - 2; bytesRead += 2)
                        {
                            rowHeader2[0] = *Buffer++;
                            rowHeader2[1] = *Buffer++;

                            switch (rowHeader2[0])
                            {
                                case 3:
                                    i = 0;
                                    for (i = 0; i < rowHeader2[1]; i++)
                                    {
                                        b = *Buffer++;
                                        sprite_frame_set_texel(Frame, column++, row, PMap->Colors[b]);
                                        /*c = PMap->Colors[b];       This variable is not used */
                                        bytesRead += 1;
                                    }

                                    if ((Buffer - ChunkInfo->Data) % 2 == 1) 
                                    { 
                                        Buffer++;
                                        bytesRead++;
                                    }
                                    break;
                                case 2:
                                    b1 = *Buffer++;
                                    /*b2 = *Buffer++;    this variable is unused */
                                    Buffer++;
                                    
                                    clr = PMap->Colors[b1];
                                    i = 0;
                                    for (i = 0; i < rowHeader2[1]; i++)
                                    {
                                        sprite_frame_set_texel(Frame, column++, row, clr);
                                    }

                                    bytesRead += 2;
                                    break;
                                case 1:
                                    column += rowHeader2[1];
                                    break;
                                default:
                                    /* Error */
                                    return 0;
                            }
                        }

                        row++;
                        break;
                    case 9:
                        i = 0;
                        for (i = 0; i < rowHeader[1]; i++)
                        {
                            row++;
                            column = 0;
                        }
                        break;
                    case 5:
                        quit = 1;
                        break;
                    default:
                        /* Error */
                        return 0;
                }

                if ((uint32_t)(Buffer - ChunkInfo->Data) == ChunkInfo->Size)
                    break;

            }
        }

#ifdef IFF2HTML
        outputPath = (char *)malloc(sizeof(char) * 255);
        
        folderName = (char *)malloc(sizeof(char) * 255);
        dummyName = (char *)malloc(sizeof(char) * 255);
        strcpy(folderName, SourceFile->FileName);
        *strchr(folderName, (int)'.') = '\0';
        sprintf(outputPath, "./%s/%s (%d) Frame %d.png", folderName, ChunkInfo->Label, ChunkInfo->ChunkID, l);
        /*printf(outputPath);
        fflush(stdout);*/
        sprite_frame_export_as_png (Frame, outputPath); /* The images export faster and look better as targa. This tells me I'm doing something wrong in the PNG exporter code */
        Sprite->Frames[l] = Frame;
        Sprite->Frames[l]->filePath = outputPath;
#endif
    }
    
    /* By adding this to the ChunkInfo at the success point, we ensure that no corrupt or partial sprite appears ingame */
    ChunkInfo->FormattedData = Sprite;
    
    return 1;
}

/* This function never returns zero because it is capable of producing logical output independent of the input's validity. */
int iff_parse_pmap(IFFChunk * ChunkInfo, uint8_t * Buffer)
{
    uint32_t i;
            
    IFFPMap *PMap = malloc(sizeof(IFFPMap));
    Buffer = ChunkInfo->Data;
    
    PMap->Colors = (IFFSpriteColor *)malloc(sizeof(IFFSpriteColor) * 256);

    Buffer += 16;
    if (*Buffer == 0)
        Buffer += 64;    /* TSO-only (this took a while to figure out) Why EA does this is beyond me... */


    /* In every single one of EA's PALT resources, there have been 256 colors. */
    for (i = 0; i < 256; i++)
    {
        if ((uint32_t)(Buffer - ChunkInfo->Data) < ChunkInfo->Size + 3)      /* Are there 3 more bytes left to read? */
        {
            PMap->Colors[i].R = *(Buffer++);
            PMap->Colors[i].G = *(Buffer++);
            PMap->Colors[i].B = *(Buffer++);
            PMap->Colors[i].A = 255;
        }
        else
        {
            PMap->Colors[i].R = 128;
            PMap->Colors[i].G = 128;
            PMap->Colors[i].B = 128;
            PMap->Colors[i].A = 255;
        }
    }
    
    ChunkInfo->FormattedData = PMap;
    
    return 1;
}

int sprite_frame_set_texel(IFFSpriteFrame *frame, uint32_t column, uint32_t row, IFFSpriteColor color)
{
    /*printf("Index: %d out of %d", frame->Width * row + column, frame->Width*frame->Height);*/
    
    memcpy(&frame->Texels[frame->Width * row + column], &color, sizeof(IFFSpriteColor));
    
    return 1;
}

int sprite_frame_export_as_targa(IFFSpriteFrame *frame, const char *filename)
{
    int i;

    FILE *targaFile = fopen(filename, "w");
    putc(0, targaFile);
    putc(0, targaFile);
    putc(2, targaFile);
    putc(0, targaFile); putc(0, targaFile);
    putc(0, targaFile); putc(0, targaFile);
    putc(0, targaFile);
    fwrite(&frame->XLocation, 2, 1, targaFile);
    fwrite(&frame->YLocation, 2, 1, targaFile);
    fwrite(&frame->Width, 2, 1, targaFile);
    fwrite(&frame->Height, 2, 1, targaFile);
    putc(32, targaFile);
    putc(0, targaFile);

    for (i = 0; i < frame->Width * frame->Height; i++)
    {
        putc(frame->Texels[i].B, targaFile);
        putc(frame->Texels[i].G, targaFile);
        putc(frame->Texels[i].R, targaFile);
        putc(frame->Texels[i].A, targaFile);
    }
    
    fclose(targaFile);
    
    return 1;
}

int sprite_frame_export_as_png(IFFSpriteFrame *frame, const char *filename)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytepp row_pointers;
    int h;
    int w;
    FILE *pngFile = fopen(filename, "wb");
    
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    info_ptr = png_create_info_struct(png_ptr);
    
    setjmp(png_jmpbuf(png_ptr));
    
    png_init_io(png_ptr, pngFile);
    
    setjmp(png_jmpbuf(png_ptr));
    
    png_set_IHDR(png_ptr, info_ptr, frame->Width, frame->Height,
                     8, 6, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
                     
    png_write_info(png_ptr, info_ptr);
    
    setjmp(png_jmpbuf(png_ptr));
    
    
    
   row_pointers = (png_bytepp) malloc (frame->Height * sizeof (png_bytep));
   for (h = 0; h < frame->Height; h++) {
      row_pointers[frame->Height-h-1] = (png_bytep) malloc (frame->Width*4);
      for (w = 0; w < frame->Width; w++) {
        row_pointers[frame->Height-h-1][w*4] = frame->Texels[frame->Width*h + w].R;
        row_pointers[frame->Height-h-1][w*4+1] = frame->Texels[frame->Width*h + w].G;
        row_pointers[frame->Height-h-1][w*4+2] = frame->Texels[frame->Width*h + w].B;
        row_pointers[frame->Height-h-1][w*4+3] = frame->Texels[frame->Width*h + w].A;
      }
   }
    
    png_write_image(png_ptr, row_pointers);
    
    setjmp(png_jmpbuf(png_ptr));
    
    png_write_end(png_ptr, NULL);
    
    for (h = 0; h < frame->Height; h++) {
      free((row_pointers)[h]);
   }
   free(row_pointers);
    
    fclose(pngFile);
    
    return 1;
}