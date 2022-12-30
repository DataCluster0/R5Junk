#include "includes.h"

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
	STUDIO_EVENT_CLIENT = 0x10000,	// Has been updated at runtime to event index on client
	STUDIO_ANIMDESC_53_UNK = 0x20000, // actually first in v52
	STUDIO_ANIMDESC_52_UNK = 0x40000,
	STUDIO_ANIMDESC_53_UNK1 = 0x80000 // cherry blossom v53, levi in v54
};

struct mstudioseqdesc_t
{
	int baseptr = 0;

	int	szlabelindex = 0;

	int szactivitynameindex = 0;

	int flags = 0; // looping/non-looping flags

	int activity = -1; // initialized at loadtime to game DLL values
	int actweight = 1;

	int numevents = 0;
	int eventindex = 0;

	Vector3 bbmin{}; // per sequence bounding box
	Vector3 bbmax{};

	int numblends = 0;

	// Index into array of shorts which is groupsize[0] x groupsize[1] in length
	int animindexindex = 0;

	int movementindex; // [blend] float array for blended movement
	int2 groupsize{ 1, 1 };
	int2 paramindex{ -1 , -1 }; // X, Y, Z, XR, YR, ZR
	Vector2 paramstart{ 0, 0 }; // local (0..1) starting value
	Vector2 paramend{ 1 , 1 }; // local (0..1) ending value
	int paramparent = 0;

	float fadeintime = 0.2; // ideal cross fate in time (0.2 default)
	float fadeouttime = 0.2; // ideal cross fade out time (0.2 default)

	int localentrynode = 0; // transition node at entry
	int localexitnode = 0; // transition node at exit
	int nodeflags = 0; // transition rules

	float entryphase = 0; // used to match entry gait
	float exitphase = 0; // used to match exit gait

	float lastframe = 0; // frame that should generation EndOfSequence

	int nextseq = 0; // auto advancing sequences
	int pose = 0; // index of delta animation between end and nextseq

	int numikrules = 0;

	int numautolayers = 0;
	int autolayerindex = 0;

	int weightlistindex = 0;

	int posekeyindex = 0;

	int numiklocks = 0;
	int iklockindex = 0;

	// Key values
	int keyvalueindex = 0;
	int keyvaluesize = 0;

	int cycleposeindex = 0; // index of pose parameter to use as cycle index

	int activitymodifierindex = 0;
	int numactivitymodifiers = 0;

	int unk = 0;
	int unk1 = 0;

	int unkindex = 0;

	int unk2 = 0;
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

	int szeventindex = 0;
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
	int sznameindex = 0;

	int unk = 0; // 0 or 1 observed.
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
	int16_t size : 12; // total size of all animation data, not next offset because even the last one has it
	int16_t flags : 4;
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
	std::vector<mstudioanimsectionsv54_t_v121> sections;

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