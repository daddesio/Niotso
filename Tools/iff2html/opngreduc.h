/*
 * opngreduc.h - libpng extension: lossless image reductions.
 *
 * Copyright (C) 2003-2011 Cosmin Truta.
 * This software is distributed under the same licensing and warranty terms
 * as libpng.
 *
 * This code is functional, although it is still work in progress.
 * Upon completion, it will be submitted for incorporation into libpng.
 */

#ifndef OPNGREDUC_H
#define OPNGREDUC_H

#include <png.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifdef PNG_INFO_IMAGE_SUPPORTED

/*
 * Indicate whether the image information is valid, i.e.
 * all the required critical information is present in the png structures.
 */
int PNGAPI opng_validate_image(png_structp png_ptr, png_infop info_ptr);

#endif /* PNG_INFO_IMAGE_SUPPORTED */


#ifndef OPNG_NO_IMAGE_REDUCTIONS
#define OPNG_IMAGE_REDUCTIONS_SUPPORTED
#endif

#ifdef OPNG_IMAGE_REDUCTIONS_SUPPORTED

#ifndef PNG_INFO_IMAGE_SUPPORTED
#error OPNG_IMAGE_REDUCTIONS_SUPPORTED requires PNG_INFO_IMAGE_SUPPORTED
#endif

#ifndef PNG_tRNS_SUPPORTED
#error OPNG_IMAGE_REDUCTIONS_SUPPORTED requires proper transparency support
#endif

/*
 * Reduce the image (bit depth + color type + palette) without
 * losing any information.  The image data must be present
 * (e.g. after calling png_set_rows(), or after loading IDAT).
 */
png_uint_32 PNGAPI opng_reduce_image(png_structp png_ptr, png_infop info_ptr,
   png_uint_32 reductions);

/*
 * PNG reduction flags.
 */
#define OPNG_REDUCE_NONE             0x0000
#define OPNG_REDUCE_16_TO_8          0x0001  /* discard bits 8-15 */
#define OPNG_REDUCE_8_TO_4_2_1       0x0002  /* discard bits 4-7, 2-7 or 1-7 */
#define OPNG_REDUCE_RGB_TO_GRAY      0x0004  /* ...also RGBA to GA */
#define OPNG_REDUCE_STRIP_ALPHA      0x0008  /* ...and create tRNS if needed */
#define OPNG_REDUCE_RGB_TO_PALETTE   0x0010  /* ...also RGBA to palette/tRNS */
#define OPNG_REDUCE_PALETTE_TO_RGB   0x0020  /* TODO */
#define OPNG_REDUCE_GRAY_TO_PALETTE  0x0040  /* ...also GA to palette/tRNS */
#define OPNG_REDUCE_PALETTE_TO_GRAY  0x0080  /* ...also palette/tRNS to GA */
#define OPNG_REDUCE_PALETTE_SLOW     0x0100  /* TODO: remove all sterile entries
                                                and reorder PLTE */
#define OPNG_REDUCE_PALETTE_FAST     0x0200  /* remove trailing sterile entries
                                                only; do not reorder PLTE */
#define OPNG_REDUCE_ANCILLARY        0x1000  /* TODO */

#define OPNG_REDUCE_BIT_DEPTH  \
   (OPNG_REDUCE_16_TO_8 | OPNG_REDUCE_8_TO_4_2_1)

#define OPNG_REDUCE_COLOR_TYPE  \
   (OPNG_REDUCE_RGB_TO_GRAY | OPNG_REDUCE_STRIP_ALPHA | \
    OPNG_REDUCE_RGB_TO_PALETTE | OPNG_REDUCE_PALETTE_TO_RGB | \
    OPNG_REDUCE_GRAY_TO_PALETTE | OPNG_REDUCE_PALETTE_TO_GRAY)

#define OPNG_REDUCE_PALETTE  \
   (OPNG_REDUCE_PALETTE_SLOW | OPNG_REDUCE_PALETTE_FAST)

#define OPNG_REDUCE_ALL  \
   (OPNG_REDUCE_BIT_DEPTH | OPNG_REDUCE_COLOR_TYPE | \
    OPNG_REDUCE_PALETTE | OPNG_REDUCE_ANCILLARY)

#endif /* OPNG_IMAGE_REDUCTIONS_SUPPORTED */


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* OPNGREDUC_H */
