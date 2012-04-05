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
int sprite_frame_set_texel_alpha(IFFSpriteFrame *frame, uint32_t column, uint32_t row, IFFSpriteColor color, uint8_t alpha);
int sprite_frame_export_as_targa(IFFSpriteFrame *frame, const char *filename);
int sprite_frame_export_as_png(IFFSpriteFrame *frame, const char *filename);

int iff_parse_sprite(IFFChunk * ChunkInfo, const uint8_t * pBuffer, IFFFile *SourceFile)
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
    uint32_t j = 0;
    
    uint32_t iRow = 0;
    uint32_t cPixelsSet = 0;
    uint8_t bQuit = 0;
    
    uint32_t cBytesReadInitial = 0;
    
    uint32_t cBytesRead = 0;
    uint16_t mRowHeader = 0;
    uint16_t mColumnHeader = 0;
    uint16_t eRowControlCode = 0;
    uint16_t eColumnControlCode = 0;
    uint16_t cBytesInThisRow = 0;
    uint16_t cPixelCount = 0;       /* Per-control-code pixel count*/
    
    uint8_t Z;
    
    
#ifdef IFF2HTML
    char *outputPath;
    char *dummyName;
    char *folderName;
#endif
    
    uint32_t version = read_uint32le(pBuffer);
	pBuffer += 4;

    if (version == 1001)
    {
        paletteID = read_uint32le(pBuffer);
		pBuffer += 4;
        frameCount = read_uint32le(pBuffer);
		pBuffer += 4;
    }
    else
    {
        frameCount = read_uint32le(pBuffer);
		pBuffer += 4;
        paletteID = read_uint32le(pBuffer);
		pBuffer += 4;
    }
    
    
    /* Try to load the appropriate palette */
    PaletteMap = iff_find_first_chunk(SourceFile, "PALT", paletteID);
    /* If that didn't work, try loading any palette from the IFF file */
    /* Some sprites in IFFs containing only one PALT don't bother to specify a correct PALT ID */
    if (PaletteMap == NULL)
    {
        printf("ERR");
        fflush(stdout);
    
        PaletteMap = iff_find_first_chunk(SourceFile, "PALT", 0);
        /* If there is no existing palette data, there can be no coherent image. */
        if (PaletteMap == NULL)
        {
            return 0;
        }
    }
    if (PaletteMap->FormattedData == NULL)
    {
        printf("HERE");
        fflush(stdout);
        iff_parse_pmap(PaletteMap, PaletteMap->Data);
    }
    PMap = (IFFPMap *)PaletteMap->FormattedData;

    offsets = (uint32_t *)malloc(sizeof(uint32_t) * frameCount);
	memset(offsets, 0, sizeof(uint32_t) * frameCount);

    if (version == 1000)
    {
        for (i = 0; i < frameCount; i++)
        {
            offsets[i] = read_uint32le(pBuffer);
            pBuffer += 4;
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
            pBuffer = ChunkInfo->Data + offsets[l];

        /* There are two "+=" statements here for clarity. That is optimized away by a decent compiler */
        if (version == 1001)
        {
            pBuffer += 4;                        /* Version */
            pBuffer += 4;                        /* Size    */
        }
        if (version != 1000 && version != 1001) /* for SPR# resources */
        {
            Frame->YLocation = read_uint16le(pBuffer);
            pBuffer += 2;
            Frame->YLocation = read_uint16le(pBuffer);
            pBuffer += 2;
            Frame->Height = read_uint16le(pBuffer);
            pBuffer += 2;
            Frame->Width = read_uint16le(pBuffer);
            pBuffer += 2;
        }
        else
        {
            Frame->Width = read_uint16le(pBuffer);
            pBuffer += 2;
            Frame->Height = read_uint16le(pBuffer);
            pBuffer += 2;
        }
        
        Frame->Flag = read_uint16le(pBuffer);
        pBuffer += 2;

        if (version == 1000 || version == 1001)
        {
            pBuffer += 2;
            Frame->PaletteID = read_uint16le(pBuffer); /* This is unused or the same as the master PALT ID */
            pBuffer += 2;
            Frame->TransparentPixel = PMap->Colors[read_uint16le(pBuffer)];
            pBuffer += 2;
            Frame->YLocation = read_uint16le(pBuffer);
            pBuffer += 2;
            Frame->XLocation = read_uint16le(pBuffer);
            pBuffer += 2;
        }
        
        
        if (Frame->PaletteID != 0xA3A3)
        {
            /* Try to load the appropriate palette */
            PaletteMap = iff_find_first_chunk(SourceFile, "PALT", Frame->PaletteID);
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
        }

        /* Now that we know the frame size, allocate the buffer to hold its texels */
        Frame->Texels = (IFFSpriteColor *)malloc(sizeof(IFFSpriteColor) * Frame->Width * Frame->Height);

        
        iRow = 0;
        cPixelsSet = 0;
        bQuit = 0;
        
        cBytesReadInitial = 0;
        
        cBytesRead = 0;
        mRowHeader = 0;
        mColumnHeader = 0;
        eRowControlCode = 0;
        eColumnControlCode = 0;
        cBytesInThisRow = 0;
        cPixelCount = 0; 
        if (version == 1000 || version == 1001)
        {
            while (!bQuit)
            {
                mRowHeader = read_uint16le(pBuffer);
                eRowControlCode = mRowHeader>>13;
                cBytesInThisRow = mRowHeader&0x1FFF;
                pBuffer += 2;
                cBytesRead = 2;                                             /* We just read the row header, which is included in cBytesInThisRow */
                cPixelsSet = 0;
                switch (eRowControlCode)
                {
                    case 0:
                        while (cBytesRead < cBytesInThisRow)
                        {
                            mColumnHeader = read_uint16le(pBuffer);
                            eColumnControlCode = mColumnHeader>>13;
                            cPixelCount = mColumnHeader&0x1FFF;
                            pBuffer += 2;
                            cBytesRead += 2;

                            
                            switch (eColumnControlCode)
                            {
                                case 1:                                     /* Add cPixelCount ZRGB pixels */
                                    
                                    for (i = 0; i < cPixelCount; i++)
                                    {
                                        Z = *pBuffer++;                     /* TODO: Use the z-buffer */
                                        sprite_frame_set_texel(Frame, cPixelsSet++, iRow, PMap->Colors[*pBuffer++]);
                                        cBytesRead += 2;
                                    }
                                    break;
                                case 2:                                     /* Add cPixelCount ZRGBA pixels */
                                    i = 0;
                                    for (i = 0; i < cPixelCount; i++)
                                    {
                                        Z = *pBuffer++;                     /* TODO: Use the z-buffer */
                                        sprite_frame_set_texel_alpha(Frame, cPixelsSet++, iRow, PMap->Colors[*pBuffer++], (uint8_t)*pBuffer++);/* Read and set the alpha channel's value */      
                                        cBytesRead += 3;
                                    }
                                                                            /* Read EA's padding byte if the current position is not a multiple of 2 */
                                    if ((pBuffer - ChunkInfo->Data) % 2 == 1) 
                                    { 
                                        pBuffer++;
                                        cBytesRead++;
                                    }
                                    break;
                                case 3:                                     /* Add cPixelCount transparent pixels */
                                    for (i = 0; i < cPixelCount; i++)
                                    {
                                        sprite_frame_set_texel(Frame, cPixelsSet++, iRow, Frame->TransparentPixel);
                                    }
                                    break;
                                case 6:                                     /* Add cPixelCount RGB pixels */
                                    i = 0;
                                    for (i = 0; i < cPixelCount; i++)
                                    {
                                        sprite_frame_set_texel(Frame, cPixelsSet++, iRow, PMap->Colors[*pBuffer++]);
                                        cBytesRead++;
                                    }
                                                                            /* Read EA's padding byte if the current position is not a multiple of 2 */
                                    if ((pBuffer - ChunkInfo->Data) % 2 == 1) 
                                    { 
                                        pBuffer++;
                                        cBytesRead++;
                                    }

                                    break;
                                default:
                                /* Error reading column code */
                                    return 0;
                            }
                        }
                                                                            /* Set any extra (unread) texels in the current row to transparent */
                        while (cPixelsSet < Frame->Width) { sprite_frame_set_texel(Frame, cPixelsSet++, iRow, Frame->TransparentPixel); }
                        iRow++;
                        
                    break;
                    
                    case 4:
                        for (i = 0; i < cBytesInThisRow; i++)               /* cBytesInThisRow is used as the count of rows to fill with the transparent color in this case */
                        {
                            for (cPixelsSet = 0; cPixelsSet < Frame->Width; cPixelsSet++)
                            {
                                sprite_frame_set_texel(Frame, cPixelsSet++, iRow, Frame->TransparentPixel);
                            }
                            iRow++;
                        }
                        break;
                    case 5:                                                 /* This means to stop reading */
                        bQuit = 1;
                        break;
                    default:
                        /* Error reading row code */
                        return 0;
                }
                
                if ((uint32_t)(pBuffer - ChunkInfo->Data) == ChunkInfo->Size)
                    break;
            }
        }
        else
        {
            while (bQuit == 0)
            {
                eRowControlCode = *pBuffer++;
                cBytesInThisRow = *pBuffer++;
                cBytesRead = 2;
                cPixelsSet = 0;
                switch (eRowControlCode)
                {
                    case 4:
                        for (cBytesInThisRow = 0; cBytesInThisRow < cBytesInThisRow;)
                        {
                            eColumnControlCode = *pBuffer++;
                            cPixelCount = *pBuffer++;
                            cBytesRead += 2;

                            switch (eColumnControlCode)
                            {
                                case 3:
                                    for (i = 0; i < cPixelCount; i++)
                                    {
                                        sprite_frame_set_texel(Frame, cPixelsSet++, iRow, PMap->Colors[*pBuffer++]);
                                        cBytesRead++;
                                    }

                                    if ((pBuffer - ChunkInfo->Data) % 2 == 1) 
                                    { 
                                        pBuffer++;
                                        cBytesRead++;
                                    }
                                    break;
                                case 2:
                                    for (i = 0; i < cPixelCount; i++)
                                    {
                                        sprite_frame_set_texel(Frame, cPixelsSet++, iRow, PMap->Colors[*pBuffer++]);
                                        pBuffer++;                                              /* Unused value */
                                        cBytesRead += 2;
                                    }
                                    break;
                                case 1:
                                    for (i = 0; i < cPixelCount; i++)
                                    {
                                        sprite_frame_set_texel(Frame, cPixelsSet++, iRow, Frame->TransparentPixel);
                                    }
                                    break;
                                default:
                                    /* Error reading column code */
                                    return 0;
                            }
                        }
                                                                            /* Set any extra (unread) texels in the current row to transparent */
                        while (cPixelsSet < Frame->Width) { sprite_frame_set_texel(Frame, cPixelsSet++, iRow, Frame->TransparentPixel); }
                        iRow++;
                        break;
                    case 9:
                        for (i = 0; i < cBytesInThisRow; i++)               /* cBytesInThisRow is used as the count of rows to fill with the transparent color in this case */
                        {
                            for (cPixelsSet = 0; cPixelsSet < Frame->Width; cPixelsSet++)
                            {
                                sprite_frame_set_texel(Frame, cPixelsSet++, iRow, Frame->TransparentPixel);
                            }
                            iRow++;
                        }
                        break;
                    case 5:
                        bQuit = 1;
                        printf("END");
                        fflush(stdout);
                        break;
                    default:
                        /* Error reading row code */
                        return 0;
                }

                /*if ((uint32_t)(pBuffer - ChunkInfo->Data) == ChunkInfo->Size)
                    break; */

            }
        }
        
        for (i = 0; i < Frame->Height; i++)               /* cBytesInThisRow is used as the count of rows to fill with the transparent color in this case */
        {
            for (j = 0; j < Frame->Width; j++)
            {
                if (sprite_are_colors_equal_rgb(Frame->Texels[i*Frame->Width+j], Frame->TransparentPixel))
                    Frame->Texels[i*Frame->Width+j].A = 0;
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
int sprite_are_colors_equal_rgb(IFFSpriteColor clr1, IFFSpriteColor clr2)
{
    return clr1.R == clr2.R && clr1.G == clr2.G && clr1.B == clr2.B;
}

/* This function never returns zero because it is capable of producing logical output independent of the input's validity. */
int iff_parse_pmap(IFFChunk * ChunkInfo, uint8_t * pBuffer)
{
    uint32_t i;
    uint32_t indicator;
    
    IFFPMap *PMap = malloc(sizeof(IFFPMap));
    pBuffer = ChunkInfo->Data;
    
    PMap->Colors = (IFFSpriteColor *)malloc(sizeof(IFFSpriteColor) * 256);

    indicator = read_uint32le(pBuffer);
    if (indicator != 1)
        pBuffer += (indicator>>24) - 8;
    else
        pBuffer += 12;

    /* In every single one of EA's PALT resources, there have been 256 colors. */
    for (i = 0; i < 256; i++)
    {
        if ((uint32_t)(pBuffer - ChunkInfo->Data) < ChunkInfo->Size + 3)      /* Are there 3 more bytes left to read? */
        {
            PMap->Colors[i].R = *(pBuffer++);
            PMap->Colors[i].G = *(pBuffer++);
            PMap->Colors[i].B = *(pBuffer++);
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

int sprite_frame_set_texel_alpha(IFFSpriteFrame *frame, uint32_t column, uint32_t row, IFFSpriteColor color, uint8_t alpha)
{
    /*printf("Index: %d out of %d", frame->Width * row + column, frame->Width*frame->Height);*/
    
    memcpy(&frame->Texels[frame->Width * row + column], &color, sizeof(IFFSpriteColor));
    frame->Texels[frame->Width * row + column].A = alpha;
    
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
      row_pointers[h] = (png_bytep) malloc (frame->Width*4);
      for (w = 0; w < frame->Width; w++) {
        row_pointers[h][w*4] = frame->Texels[frame->Width*h + w].R;
        row_pointers[h][w*4+1] = frame->Texels[frame->Width*h + w].G;
        row_pointers[h][w*4+2] = frame->Texels[frame->Width*h + w].B;
        row_pointers[h][w*4+3] = frame->Texels[frame->Width*h + w].A;
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