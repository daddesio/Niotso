#include "libvitaboy.hpp"

VBFile_t VBFile;

void ReadPropEntry(KeyValuePair_t& Entry){
    Entry.Key = VBFile.readstring();
    printf(" | | | | | Key: %s\n", Entry.Key);
    Entry.Value = VBFile.readstring();
    printf(" | | | | | Value: %s\n", Entry.Value);
}

void ReadPropEntries(Prop_t& Prop){
    unsigned count = Prop.EntriesCount = VBFile.readint32();
    printf(" | | | | EntriesCount: %u\n", Prop.EntriesCount);
    Prop.Entries = (KeyValuePair_t*) malloc(count * sizeof(KeyValuePair_t));
    
    for(unsigned i=0; i<count; i++){
        printf(" | | | | [Entry %u]\n", i+1);
        ReadPropEntry(Prop.Entries[i]);
    }
}

void FindQuaternionMatrix(float * Matrix, Rotation_t * Quaternion){
	float x2 = Quaternion->x * Quaternion->x;
	float y2 = Quaternion->y * Quaternion->y;
	float z2 = Quaternion->z * Quaternion->z;
	float xy = Quaternion->x * Quaternion->y;
	float xz = Quaternion->x * Quaternion->z;
	float yz = Quaternion->y * Quaternion->z;
	float wx = Quaternion->w * Quaternion->x;
	float wy = Quaternion->w * Quaternion->y;
	float wz = Quaternion->w * Quaternion->z;
 
    Matrix[0] = 1.0f - 2.0f * (y2 + z2);
    Matrix[1] = 2.0f * (xy - wz);
    Matrix[2] = 2.0f * (xz + wy);
    Matrix[3] = 0.0f;
    Matrix[4] = 2.0f * (xy + wz);
    Matrix[5] = 1.0f - 2.0f * (x2 + z2);
    Matrix[6] = 2.0f * (yz - wx);
    Matrix[7] = 0.0f;
    Matrix[8] = 2.0f * (xz - wy);
    Matrix[9] = 2.0f * (yz + wx);
    Matrix[10] = 1.0f - 2.0f * (x2 + y2);
    Matrix[11] = 0.0f;
    Matrix[12] = 0.0f;
    Matrix[13] = 0.0f;
    Matrix[14] = 0.0f;
    Matrix[15] = 1.0f;
}