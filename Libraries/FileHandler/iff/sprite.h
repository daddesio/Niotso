#define IFF2HTML

typedef struct IFFSpriteColor_struct
{
    uint8_t A;
    uint8_t R;
    uint8_t G;
    uint8_t B;
} IFFSpriteColor;

typedef struct IFFPixelMap_struct
{
    IFFSpriteColor *Colors;                  /* This is 255 b/c sometimes SPR2 and SPR resource go out of bounds (safety first!) */
} IFFPMap;

typedef struct IFFSpriteFrame_struct
{
    uint16_t XLocation;
    uint16_t YLocation;
    uint16_t Width;
    uint16_t Height;
    uint16_t Flag;
    uint16_t PaletteID;
    IFFSpriteColor TransparentPixel;
    
    IFFSpriteColor *Texels;
    
#ifdef IFF2HTML
    char *filePath;
#endif

} IFFSpriteFrame;

typedef struct IFFSprite_struct
{
    IFFSpriteFrame **Frames;
    uint16_t FrameCount;
#ifdef IFF2HTML
    uint32_t Version;
#endif
} IFFSprite;
