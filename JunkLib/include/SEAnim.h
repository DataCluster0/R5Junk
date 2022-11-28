#pragma once
#include <includes.h>

#define SEANIM_VERSION			1
#define SEANIM_MAGIC			"SEAnim"

// Specifies how the data is interpreted by the importer
enum SEAnim_AnimationType : uint8_t
{
	// Animation translations are set to this exact value each frame
	SEANIM_TYPE_ABSOLUTE,
	// This animation is applied to existing animation data in the scene
	SEANIM_TYPE_ADDITIVE,
	// Animation translations are based on rest position in scene
	SEANIM_TYPE_RELATIVE,
	// This animation is relative and contains delta data (Whole model movement) Delta tag name must be set!
	SEANIM_TYPE_DELTA
};

// Specifies the data present for each frame of every bone
enum SEAnim_DataPresenceFlags : uint8_t
{
	// These describe what type of keyframe data is present for the bones
	SEANIM_BONE_LOC = 1 << 0,
	SEANIM_BONE_ROT = 1 << 1,
	SEANIM_BONE_SCALE = 1 << 2,

	// If any of the above flags are set, then bone keyframe data is present, thus this comparing against this mask will return true
	SEANIM_PRESENCE_BONE = SEANIM_BONE_LOC | SEANIM_BONE_ROT | SEANIM_BONE_SCALE,

	// The file contains notetrack data
	SEANIM_PRESENCE_NOTE = 1 << 6,
	// The file contains a custom data block
	SEANIM_PRESENCE_CUSTOM = 1 << 7,
};

enum SEAnim_BoneFlags : uint8_t
{
	SEANIM_BONE_NORMAL = 0,
	SEANIM_BONE_COSMETIC = 1 << 0
};

enum SEAnim_PropertyFlags : uint8_t
{
	SEANIM_PRECISION_HIGH = 1 << 0 // Use double precision floating point vectors instead of single precision
};

enum SEAnim_Flags
{
	SEANIM_LOOPED = 1 << 0, // The animation is a looping animation
};

struct SEAnim_BoneAnimModifier_t
{
	uint32_t index; // Index of the bone
	uint8_t animTypeOverride; // AnimType to use for that bone, and its children recursively
};

struct SEAnim_BonePosData_t
{
	uint32_t frame = 0;
	Vector3 pos{};
};

struct SEAnim_BoneRotData_t
{
	uint32_t frame = 0;
	Quaternion rot{};
};

struct SEAnim_BoneScaleData_t
{
	uint32_t frame = 0;
	Vector3 scale{}; // 1.0 is the default scale, 2.0 is twice as big, and 0.5 is half size
};

struct SEAnim_Note_t
{
	uint32_t frame;
	char* name;
};

struct SEAnim_BoneData_t
{
	uint8_t flags = 0;

	std::vector<SEAnim_BonePosData_t> poslist{};
	std::vector<SEAnim_BoneRotData_t> rotlist{};
	std::vector<SEAnim_BoneScaleData_t> scalelist{};
};

struct SEAnim_Header_t
{
	// Contains the size of the header block in bytes, any extra data is ignored
	uint16_t headerSize; //currently 0x1C

	// The type of animation data that is stored, matches an SEANIM_TYPE
	uint8_t animType;

	// Bitwise flags that define the properties for the animation itself
	uint8_t animFlags;

	// Bitwise flags that define which data blocks are present, and properties for those data blocks
	uint8_t dataPresenceFlags;

	// Bitwise flags containing property information pertaining regarding the data in the file
	uint8_t dataPropertyFlags;

	// RESERVED - Should be 0
	uint8_t reserved1[2];

	float framerate;

	// frameCount describes the length of the animation (in frames)
	// It is used to determine the size of frame_t
	// This should be equal to the largest frame number in the anim (Including keys & notes) plus 1
	// Ex: An anim with keys on frame 0, 5 and 8 - frameCount would be 9
	//     An anim with keys on frame 4, 7, and 14 - frameCount would be 15
	uint32_t frameCount;

	// Is 0 if ( presenceFlags & SEANIM_PRESENCE_BONE ) is false
	uint32_t boneCount;
	uint8_t boneAnimModifierCount; // The number of animType modifier bones

	// RESERVED - Should be 0
	uint8_t reserved2[3];

	// Is 0 if ( presenceFlags & SEANIM_PRESENCE_NOTE ) is false
	uint32_t noteCount;
};

struct SEAnim_File_Header_t
{
	char magic[6];		// 'SEAnim'
	uint16_t version;	// The file version - the current version is 0x1
	SEAnim_Header_t header;
};

struct SEAnim_File_t
{
	char magic[6];		// 'SEAnim'
	uint16_t version;	// The file version - the current version is 0x1
	SEAnim_Header_t header;

	uint8_t** bone;
	SEAnim_BoneAnimModifier_t* boneModifiers;
	SEAnim_BoneData_t* boneData;

	uint32_t				noteCount;
	SEAnim_Note_t* notes;

	uint32_t customDataBlockSize;
	uint8_t* customDataBlockBuf;
};

struct SEAnimBoneFrame_o
{
	Vector3 pos{};
	Quaternion rot{};
	Vector3 scale{}; // 1.0 is the default scale, 2.0 is twice as big, and 0.5 is half size
};

struct SEAnimBone_o
{
	std::string Name = "";

	int flags = 0;

	std::vector<SEAnimBoneFrame_o> Frames{};
};

int GetBoneTSize(int bonecount);

uint32_t ReadBoneT(BinaryIO& reader, int bonecount);

std::vector<SEAnimBone_o> ReadSEAnim(BinaryIO& reader);