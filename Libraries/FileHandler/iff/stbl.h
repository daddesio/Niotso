/* STR# chunk */

enum IFFLanguage {
    IFFLANG_DEFAULT             = 0,
    IFFLANG_EN_US               = 1,
    IFFLANG_EN_INTERNATIONAL    = 2,
    IFFLANG_FRENCH              = 3,
    IFFLANG_GERMAN              = 4,
    IFFLANG_ITALIAN             = 5,
    IFFLANG_SPANISH             = 6,
    IFFLANG_DUTCH               = 7,
    IFFLANG_DANISH              = 8,
    IFFLANG_SWEDISH             = 9,
    IFFLANG_NORWEGIAN           = 10,
    IFFLANG_FINNISH             = 11,
    IFFLANG_HEBREW              = 12,
    IFFLANG_RUSSIAN             = 13,
    IFFLANG_PORTUGUESE          = 14,
    IFFLANG_JAPANESE            = 15,
    IFFLANG_POLISH              = 16,
    IFFLANG_CHINESE_SIMPLIFIED  = 17,
    IFFLANG_CHINESE_TRADITIONAL = 18,
    IFFLANG_THAI                = 19,
    IFFLANG_KOREAN              = 20
};

typedef struct IFFStringPair_struct
{
    uint8_t LanguageSet;
    char * Key;
    char * Value;
} IFFStringPair;

typedef struct IFFStringPairNode_struct
{
    IFFStringPair Pair;
    struct IFFStringPairNode_struct * PrevPair;
    struct IFFStringPairNode_struct * NextPair;
} IFFStringPairNode;

typedef struct IFFLanguageSet_struct
{
    uint16_t PairCount;
    IFFStringPairNode * FirstPair;
    IFFStringPairNode * LastPair;
} IFFLanguageSet;

typedef struct IFF_STR_struct
{
    int16_t Format;
    IFFLanguageSet LanguageSets[20];
} IFF_STR;