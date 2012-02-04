#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>

class VBFile_t {
  private:
    const uint8_t *Buffer, *Position;
    unsigned Size;
    
  public:
    inline void set(const void *_Buffer, unsigned _Size){
        Buffer = (const uint8_t*) _Buffer;
        Position = (const uint8_t*) _Buffer;
        Size = _Size;
    }
    
    inline unsigned getpos(){
        return Position-Buffer;
    }
    
    inline void seekto(unsigned offset){
        Position = Buffer+offset;
    }
    
    inline uint32_t readint32(){
        uint32_t value = (uint32_t)((Position[0]<<(8*3)) | (Position[1]<<(8*2)) | (Position[2]<<(8*1)) | (Position[3]<<(8*0)));
        Position += 4;
        return value;
    }
    
    inline uint32_t readint16(){
        uint16_t value = (uint16_t)((Position[0]<<(8*1)) | (Position[1]<<(8*0)));
        Position += 2;
        return value;
    }
    
    inline uint32_t readint8(){
        uint8_t value = (uint8_t)((Position[0]<<(8*0)));
        Position += 1;
        return value;
    }
    
    inline float readfloat(){
        //Obviously a platform-dependent implementation
        float value;
        memcpy(&value, Position, 4);
        Position += 4;
        return value;
    }
    
    inline void readbytes(void* Destination, unsigned length){
        memcpy(Destination, Position, length);
        Position += length;
    }
    
    inline char* readstring(){
        //Read a Pascal string with 1 length byte
        unsigned length = readint8();
        char *string = (char*) malloc((length+1) * sizeof(char));
        readbytes(string, length);
        string[length] = '\0';
        return string;
    }
    
    inline char* readstring2(){
        //Read a Pascal string with 2 length bytes
        unsigned length = readint16();
        char *string = (char*) malloc((length+1) * sizeof(char));
        readbytes(string, length);
        string[length] = '\0';
        return string;
    }
};

extern VBFile_t VBFile;

struct Translation_t {
    float x, y, z;
};

struct Rotation_t {
    float w, x, y, z;
};

struct KeyValuePair_t {
    char * Key;
    char * Value;
};

struct Prop_t {
    uint32_t EntriesCount;
    KeyValuePair_t * Entries;
};

struct PropsList_t {
    uint32_t PropsCount;
    Prop_t * Props;
};

struct TimeProp_t {
    uint32_t ID;
    PropsList_t PropsList;
};

struct TimePropsList_t {
    uint32_t TimePropsCount;
    TimeProp_t * TimeProps;
};

struct Motion_t {
    uint32_t Unknown;
    char * BoneName;
    uint32_t FrameCount;
    float Duration;
    uint8_t HasTranslation;
    uint8_t HasRotation;
    uint32_t TranslationsOffset;
    uint32_t RotationsOffset;
    Translation_t * Translations;
    Rotation_t * Rotations;

    uint8_t HasPropsLists;
    uint32_t PropsListsCount;
    PropsList_t * PropsLists;

    uint8_t HasTimePropsLists;
    uint32_t TimePropsListsCount;
    TimePropsList_t * TimePropsLists;
};

struct Animation_t {
    uint32_t Version;
    char * Name;
    float Duration;
    float Distance;
    uint8_t IsMoving;
    uint32_t TranslationsCount;
    uint32_t RotationsCount;
    uint32_t MotionsCount;

    unsigned TranslationsOffset;
    unsigned RotationsOffset;

    Motion_t * Motions;
};

void ReadPropEntry(KeyValuePair_t& Entry);
void ReadPropEntries(Prop_t& Prop);
void ReadAnimation(Animation_t& Animation);
void ReadMotion(Animation_t& Animation, Motion_t& Motion);
void ReadPropsList(PropsList_t& PropsList);
void ReadPropsLists(Motion_t& Motion);
void ReadTimePropsList(TimePropsList_t& TimePropsList);
void ReadTimePropsLists(Motion_t& Motion);