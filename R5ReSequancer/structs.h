#include "includes.h"

struct Vector2
{
	float x, y;

	Vector2(float x, float y) {
		this->x = x;
		this->y = y;
	}

	Vector2() {};
};

struct Vector3
{
	float x, y, z;

	Vector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3() {};
};

struct Quaternion
{
	float x, y, z, w;

	Quaternion(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Quaternion() {};
};

// packed vector
struct Vector32
{
	uint32_t x : 10;
	uint32_t y : 10;
	uint32_t z : 10;
	uint32_t exp : 2;
};

struct Vector48
{
	int x : 2;
	int y : 2;
	int z : 2;
};

struct Quat32
{
	uint32_t x : 11;
	uint32_t y : 10;
	uint32_t z : 10;
	uint32_t wneg : 1;
};

struct Quat48
{
	uint16_t x : 16;
	uint16_t y : 16;
	uint16_t z : 15; // broken
	uint16_t wneg : 1;
};

struct Quat64
{
	uint64_t x : 21;
	uint64_t y : 21;
	uint64_t z : 21;
	uint64_t wneg : 1;

	//  <read=ReadShort(sqrt( 1 - x * x - y * y - z * z ))>
};

enum mstudioseqflags : uint32_t
{
	STUDIO_LOOPING = 0x0001,	// ending frame should be the same as the starting frame
	STUDIO_SNAP = 0x0002,	// do not interpolate between previous animation and this one
	STUDIO_DELTA = 0x0004,	// this sequence "adds" to the base sequences, not slerp blends
	STUDIO_AUTOPLAY = 0x0008,	// temporary flag that forces the sequence to always play
	STUDIO_POST = 0x0010,	//
	STUDIO_ALLZEROS = 0x0020,	// this animation/sequence has no real animation data
	STUDIO_FRAMEANIM = 0x0040,	// animation is encoded as by frame x bone instead of RLE bone x frame
	STUDIO_CYCLEPOSE = 0x0080,	// cycle index is taken from a pose parameter index
	STUDIO_REALTIME = 0x0100,	// cycle index is taken from a real-time clock, not the animations cycle index
	STUDIO_LOCAL = 0x0200,	// sequence has a local context sequence
	STUDIO_HIDDEN = 0x0400,	// don't show in default selection views
	STUDIO_OVERRIDE = 0x0800,	// a forward declared sequence (empty)
	STUDIO_ACTIVITY = 0x1000,	// Has been updated at runtime to activity index
	STUDIO_EVENT = 0x2000,	// Has been updated at runtime to event index on server
	STUDIO_WORLD = 0x4000,	// sequence blends in worldspace
	STUDIO_NOFORCELOOP = 0x8000,	// do not force the animation loop
	STUDIO_EVENT_CLIENT = 0x10000	// Has been updated at runtime to event index on client
};

struct mstudioseqdesc_t
{
	int baseptr;

	int	szlabelindex;

	int szactivitynameindex;

	int flags; // looping/non-looping flags

	int activity; // initialized at loadtime to game DLL values
	int actweight;

	int numevents;
	int eventindex;

	Vector3 bbmin; // per sequence bounding box
	Vector3 bbmax;

	int numblends;

	// Index into array of shorts which is groupsize[0] x groupsize[1] in length
	int animindexindex;

	int movementindex; // [blend] float array for blended movement
	int groupsize[2];
	int paramindex[2]; // X, Y, Z, XR, YR, ZR
	float paramstart[2]; // local (0..1) starting value
	float paramend[2]; // local (0..1) ending value
	int paramparent;

	float fadeintime; // ideal cross fate in time (0.2 default)
	float fadeouttime; // ideal cross fade out time (0.2 default)

	int localentrynode; // transition node at entry
	int localexitnode; // transition node at exit
	int nodeflags; // transition rules

	float entryphase; // used to match entry gait
	float exitphase; // used to match exit gait

	float lastframe; // frame that should generation EndOfSequence

	int nextseq; // auto advancing sequences
	int pose; // index of delta animation between end and nextseq

	int numikrules;

	int numautolayers;
	int autolayerindex;

	int weightlistindex;

	int posekeyindex;

	int numiklocks;
	int iklockindex;

	// Key values
	int keyvalueindex;
	int keyvaluesize;

	int cycleposeindex; // index of pose parameter to use as cycle index

	int activitymodifierindex;
	int numactivitymodifiers;

	int unk;
	int unk1;

	int unkindex;

	int unk2;
};

struct mstudioanimdescv54_t
{
	int baseptr;

	int sznameindex;

	float fps; // frames per second
	int flags; // looping/non-looping flags

	int numframes;

	// piecewise movement
	int nummovements;
	int movementindex;

	int compressedikerrorindex;
	int animindex; // non-zero when anim data isn't in sections

	int numikrules;
	int ikruleindex; // non-zero when IK data is stored in the mdl

	int sectionindex;
	int sectionframes; // number of frames used in each fast lookup section, zero if not used
};

// rseq v7.1
struct mstudioanimdescv54_t_v121
{
	int baseptr;

	int sznameindex;

	float fps; // frames per second
	int flags; // looping/non-looping flags

	int numframes;

	// piecewise movement
	int nummovements;
	int movementindex;

	int compressedikerrorindex;
	int animindex; // non-zero when anim data isn't in sections

	int numikrules;
	int ikruleindex; // non-zero when IK data is stored in the mdl

	int sectionindex;

	int unk; // what, obviously section related as it's wedged between sectionindex and sectiom frames

	int sectionframes; // number of frames used in each fast lookup section, zero if not used

	int unk1; // Padding
	int unk2; // Padding
	int unk3; // SomeDataOffset
	int unk4; // SomeDataOffset

	// it seems like there's another int here but I'm unsure
};

struct mstudioeventv54_t
{
	float cycle = 0.0;
	int	Event = 0;
	int type = 0x400; // this will be 0 if old style I'd imagine
	char options[256];

	int szeventindex;
};

// rseq v10
struct mstudioeventv54_t_v122
{
	float cycle;
	int	Event;
	int type; // this will be 0 if old style I'd imagine

	int unk;

	char options[256]; // this is the only difference compared to normal v54

	int szeventindex;
};

struct mstudioautolayer_t
{
	uint64_t guid; // hashed aseq guid asset

	short iSequence;
	short iPose;

	int flags;
	float start; // beginning of influence
	float peak;	 // start of full influence
	float tail;	 // end of full influence
	float end;	 // end of all influence
};

enum ikruletype : uint32_t
{
	IK_SELF = 1,
	IK_WORLD = 2,
	IK_GROUND = 3,
	IK_RELEASE = 4,
	IK_ATTACHMENT = 5,
	IK_UNLATCH = 6
};

struct mstudioikrulev54_t
{
	int index;
	ikruletype type;
	int chain;
	int bone;

	int slot; // iktarget slot. Usually same as chain.
	float height;
	float radius;
	float floor;
	Vector3 pos;
	Quaternion q;

	// in later versions this is scale, that also might apply here
	int unk1[7]; // might be before pos and q

	int compressedikerrorindex = 0;

	int iStart = 0;
	int ikerrorindex = 0;

	float start = 0; // beginning of influence
	float peak = 0; // start of full influence
	float tail = 0; // end of full influence
	float end = 0; // end of all influence

	float contact = 0; // frame footstep makes ground concact
	float drop = 0; // how far down the foot should drop when reaching for IK
	float top = 0; // top of the foot box

	int szattachmentindex = 0; // name of world attachment

	int unk2 = 0; // there is an int here in apex and 9 unused doesn't make sense.
};

struct mstudioikrulev54_t_new
{
	int index;
	ikruletype type;
	int chain;
	int bone;

	int slot; // iktarget slot. Usually same as chain.
	float height;
	float radius;
	float floor;
	Vector3 pos;
	Quaternion q;

	float scale[6]; // these values are the same as what posscale (if it was used) and rotscale are.
	//int16 offset[6];

	int unk1[3];

	//int compressedikerrorindex;

	int iStart;
	//int ikerrorindex;

	float start; // beginning of influence
	float peak; // start of full influence
	float tail; // end of full influence
	float end; // end of all influence

	float contact; // frame footstep makes ground concact
	float drop; // how far down the foot should drop when reaching for IK
	float top; // top of the foot box

	int szattachmentindex; // name of world attachment

	int unk2;

	//int unused[9];
};

struct mstudiocompressedikerrorv54_t
{
	float scale[4]; // first three values are the same as what posscale (if it was used) is, fourth is similar to unkvector1.
	// unknown how this works now
	//int offset[9];
};

struct mstudioikrulezeroframe_t
{
	uint16_t		chain;
	uint16_t		slot;
	float		start;	// beginning of influence
	float		peak;	// start of full influence
	float		tail;	// end of full influence
	float		end;	// end of all influence
};

struct mstudioautolayerv54_t
{
	uint64_t guid; // hashed aseq guid asset

	short iSequence;
	short iPose;

	int flags;
	float start;	// beginning of influence
	float peak;	// start of full influence
	float tail;	// end of full influence
	float end;	// end of all influence
};

enum eventtype
{
	NEW_EVENT_STYLE = (1 << 10),
};

struct mstudioactivitymodifierv53_t
{
	int sznameindex;

	int unk; // 0 or 1 observed.
};

struct animflagarrayv54_t
{
	// don't think this is correct
	uint8_t STUDIO_ANIM_POS_54 : 1;
	uint8_t STUDIO_ANIM_ROT_54 : 1;
	uint8_t STUDIO_ANIM_SCALE_54 : 1;
	uint8_t STUDIO_ANIM_UNK_54 : 1;
};

struct mstudioanim_valueptrv54_t
{
	//int ok;

	int test : 8;
	int test3 : 3;
	int test1 : 8;
	int test5 : 3;
	int test4 : 8;
};

// used for piecewise loading of animation data
struct mstudioanimblock_t
{
	int datastart;
	int dataend;
};

struct mstudioanimsectionsv53_t
{
	int animindex;
};

struct mstudioanimsectionsv54_t_v121
{
	int animindex;
	int isExternal; // 0 or 1, if 1 section is not in rseq (I think)
};


struct mstudio_rle_animv54_t
{
	uint16_t _tsize : 12; // total size of all animation data, not next offset because even the last one has it
	uint16_t flags : 4;
};

#define STUDIO_ANIM_DELTA_54	 0x01 // unverified
#define STUDIO_ANIM_ANIMSCALE_54 0x02 // mstudioanim_valueptr_t
#define STUDIO_ANIM_ANIMROT_54	 0x04 // mstudioanim_valueptr_t
#define STUDIO_ANIM_ANIMPOS_54	 0x08 // mstudioanim_valueptr_t

struct ASEQ_Out_event
{
	mstudioeventv54_t event;
	std::string string;
};

struct ASEQ_Out_modifier
{
	mstudioactivitymodifierv53_t activitymodifier;
	std::string string;
};

struct ASEQ_Out_animdesc
{
	mstudioanimdescv54_t desc;
	std::vector<mstudioikrulev54_t> ikrules;
	std::vector<animflagarrayv54_t> flagarray;
	std::vector<mstudioanimsectionsv53_t> sections;

	char* animationdata;
};

struct ASEQ_Out
{
	mstudioseqdesc_t hdr;

	std::string szlabel;
	std::string szactivity;

	std::vector<float> posekeys;

	std::vector<ASEQ_Out_event> events;

	std::vector<mstudioautolayerv54_t> autolayers;

	std::vector<float> weightlist;
	std::vector<ASEQ_Out_modifier> activitymodifiers;

	std::vector<ASEQ_Out_animdesc> animdescs;

	std::vector<int> blendgroups;
};

size_t Read_mstudio_rle_anim_t(BinaryIO& reader, animflagarrayv54_t boneflag)
{
	size_t size = 0;

	auto animv54 = reader.read<mstudio_rle_animv54_t>();

	size += sizeof(mstudio_rle_animv54_t);

	if (animv54.flags & STUDIO_ANIM_ANIMPOS_54)
		float posscale = reader.read<float>();

	if (boneflag.STUDIO_ANIM_POS_54)
	{
		if (animv54.flags & STUDIO_ANIM_ANIMPOS_54)
			auto animpos = reader.read<mstudioanim_valueptrv54_t>();
		else
			Vector48 rawpos = reader.read<Vector48>();
	}

	if (boneflag.STUDIO_ANIM_ROT_54)
	{
		if (animv54.flags & STUDIO_ANIM_ANIMROT_54)
			auto animrot = reader.read<mstudioanim_valueptrv54_t>();
		else
			Quat64 rawrot = reader.read<Quat64>();
	}

	if (boneflag.STUDIO_ANIM_SCALE_54)
	{
		if (animv54.flags & STUDIO_ANIM_ANIMSCALE_54)
			auto animscale = reader.read<mstudioanim_valueptrv54_t>();
		else
			Vector48 rawscale = reader.read<Vector48>();
	}

	return animv54._tsize;
};