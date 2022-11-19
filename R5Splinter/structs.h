#pragma once
#include <includes.h>
using namespace std;

// modified source engine studio mdl header struct
struct studiohdr_t
{
	int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
	int version; // Format version number, such as 48 (0x30,0x00,0x00,0x00)
	int checksum; // This has to be the same in the phy and vtx files to load!
	int sznameindex; // This has been moved from studiohdr2 to the front of the main header.

	char name[64]; // The internal name of the model, padding with null bytes.
	// Typically "my_model.mdl" will have an internal name of "my_model"
	int length; // Data size of MDL file in bytes.

	Vector3 eyeposition;	// ideal eye position

	Vector3 illumposition;	// illumination center

	Vector3 hull_min;		// ideal movement hull size
	Vector3 hull_max;

	Vector3 view_bbmin;		// clipping bounding box
	Vector3 view_bbmax;

	int flags;

	int numbones; // bones
	int boneindex;

	int numbonecontrollers; // bone controllers
	int bonecontrollerindex;

	int numhitboxsets;
	int hitboxsetindex;

	int numlocalanim; // animations/poses
	int localanimindex; // animation descriptions

	int numlocalseq; // sequences
	int	localseqindex;

	int activitylistversion; // initialization flag - have the sequences been indexed?

	// mstudiotexture_t
	// short rpak path
	// raw textures
	int materialtypesindex;
	int numtextures; // the material limit exceeds 128, probably 256.
	int textureindex;

	// this should always only be one, unless using vmts.
	// raw textures search paths
	int numcdtextures;
	int cdtextureindex;

	// replaceable textures tables
	int numskinref;
	int numskinfamilies;
	int skinindex;

	int numbodyparts;
	int bodypartindex;

	int numlocalattachments;
	int localattachmentindex;

	int numlocalnodes;
	int localnodeindex;
	int localnodenameindex;

	int numflexdesc;
	int flexdescindex;

	int meshindex; // SubmeshLodsOffset, might just be a mess offset

	int numflexcontrollers;
	int flexcontrollerindex;

	int numflexrules;
	int flexruleindex;

	int numikchains;
	int ikchainindex;

	// this is rui meshes
	int numruimeshes;
	int ruimeshindex;

	int numlocalposeparameters;
	int localposeparamindex;

	int surfacepropindex;

	int keyvalueindex;
	int keyvaluesize;

	int numlocalikautoplaylocks;
	int localikautoplaylockindex;

	float mass;
	int contents;

	// unused for packed models
	int numincludemodels;
	int includemodelindex;

	uint32_t virtualModel;

	int bonetablebynameindex;

	// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
	// this value is used to calculate directional components of lighting
	// on static props
	uint8_t constdirectionallightdot;

	// set during load of mdl data to track *desired* lod configuration (not actual)
	// the *actual* clamped root lod is found in studiohwdata
	// this is stored here as a global store to ensure the staged loading matches the rendering
	uint8_t rootLOD;

	// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
	// to be set as root LOD:
	//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
	//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
	uint8_t numAllowedRootLODs;

	uint8_t unused = 0;

	float fadedistance;

	float gathersize; // what. from r5r struct

	int numunk_v54_early = 0;
	int unkindex_v54_early = 0;

	int unk_v54[2];

	// this is in all shipped models, probably part of their asset bakery. it should be 0x2CC.
	int mayaindex; // doesn't actually need to be written pretty sure, only four bytes when not present.

	int numsrcbonetransform;
	int srcbonetransformindex;

	int	illumpositionattachmentindex;

	int linearboneindex;

	int m_nBoneFlexDriverCount; // unsure if that's what it is in apex
	int m_nBoneFlexDriverIndex;

	int unkindexflex;

	// aabb tree in here maybe? definitely unused
	int unk1_v54[6];

	// always "" or "Titan"
	int unkstringindex;

	// this is now used for combined files in rpak, vtx, vvd, and vvc are all combined while vphy is separate.
	// the indexes are added to the offset in the rpak mdl_ header.
	// vphy isn't vphy, looks like a heavily modified vphy.
	int vtxindex; // VTX
	int vvdindex; // VVD / IDSV
	int vvcindex; // VVC / IDCV
	int vphyindex; // VPHY / IVPS

	int vtxsize;
	int vvdsize;
	int vvcsize;
	int vphysize; // still used in models using vg

	// unk2_v54[3] is the chunk after following unkindex2's chunk
	int unk2_v54[3]; // the same four unks in v53 I think, the first index being unused now probably

	int unkindex3; // index to chunk after string block

	// likely related to AABB
	Vector3 mins; // min/max for Something
	Vector3 maxs; // seem to be the same as hull size

	int unk3_v54[3];

	int unkindex4; // chunk before unkindex3 sometimes

	int unk4_v54[3]; // same as unk3_v54_v121

	//int vgindex; // 0tVG
	//int unksize; // might be offset
	//int unksize1; // might be offset
};

enum eBoneFlags : uint32_t
{
	BONE_CALCULATE_MASK = 0x1F,
	BONE_PHYSICALLY_SIMULATED = 0x01,	// bone is physically simulated when physics are active
	BONE_PHYSICS_PROCEDURAL = 0x02,	// procedural when physics is active
	BONE_ALWAYS_PROCEDURAL = 0x04,	// bone is always procedurally animated
	BONE_SCREEN_ALIGN_SPHERE = 0x08,	// bone aligns to the screen, not constrained in motion.
	BONE_SCREEN_ALIGN_CYLINDER = 0x10,	// bone aligns to the screen, constrained by it's own axis.

	BONE_WORLD_ALIGN = 0x20, // bone is rigidly aligned to the world (but can still translate)(added in CSGO)

	BONE_USED_BY_IKCHAIN = 0x20, // bone is influenced by IK chains, added in V52 (Titanfall 1)

	BONE_USED_MASK = 0x0007FF00,
	BONE_USED_BY_ANYTHING = 0x0007FF00,
	BONE_USED_BY_HITBOX = 0x00000100,	// bone (or child) is used by a hit box
	BONE_USED_BY_ATTACHMENT = 0x00000200,	// bone (or child) is used by an attachment point
	BONE_USED_BY_VERTEX_MASK = 0x0003FC00,
	BONE_USED_BY_VERTEX_LOD0 = 0x00000400,	// bone (or child) is used by the toplevel model via skinned vertex
	BONE_USED_BY_VERTEX_LOD1 = 0x00000800,
	BONE_USED_BY_VERTEX_LOD2 = 0x00001000,
	BONE_USED_BY_VERTEX_LOD3 = 0x00002000,
	BONE_USED_BY_VERTEX_LOD4 = 0x00004000,
	BONE_USED_BY_VERTEX_LOD5 = 0x00008000,
	BONE_USED_BY_VERTEX_LOD6 = 0x00010000,
	BONE_USED_BY_VERTEX_LOD7 = 0x00020000,
	BONE_USED_BY_BONE_MERGE = 0x00040000,	// bone is available for bone merge to occur against it

	BONE_FLAG_UNK_53 = 0x00080000,

	//#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
	//#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

	BONE_TYPE_MASK = 0x00F00000,
	BONE_FIXED_ALIGNMENT = 0x00100000,	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

	BONE_HAS_SAVEFRAME_POS = 0x00200000,	// Vector48
	BONE_HAS_SAVEFRAME_ROT64 = 0x00400000,	// Quaternion64
	BONE_HAS_SAVEFRAME_ROT32 = 0x00800000,	// Quaternion32
};

struct mstudiobonev54_t
{
	uint32_t sznameindex = 0;

	uint32_t parent = 0xFFFFFFF; // parent bone
	uint32_t bonecontroller[6]; // bone controller index, -1 == none

	// default values
	Vector3 pos{};
	Quaternion quat{};
	RadianEuler rot{};

	Vector3 unkvector{}; // the same as whatever v53 is

	matrix3x4_t poseToBone{};
	Quaternion qAlignment{};

	uint32_t flags = 0;
	uint32_t proctype = 0;
	uint32_t procindex = 0; // procedural rule
	uint32_t physicsbone = 0; // index into physically simulated bone

	uint32_t surfacepropidx = 0; // index into string tablefor property name

	uint32_t contents = 0; // See BSPFlags.h for the contents flags

	uint32_t surfacepropLookup = 0; // written on compile in v54

	uint32_t unk = 0;

	uint32_t unkid = 0; // id is for unk section after stringblock, lacks toggle
};

struct mstudiolinearbonev54_t
{
	// they cut pos and rot scale, understandable since posscale was never used it tf|2 and they do anims different in apex
	uint32_t numbones = 0;
	uint32_t flagsindex = 0;
	uint32_t parentindex = 0;
	uint32_t posindex = 0;
	uint32_t quatindex = 0;
	uint32_t rotindex = 0;
	uint32_t posetoboneindex = 0;
};

struct mstudiojigglebonev54_t
{
	uint8_t flags; // looks to be.

	uint8_t bone; // id of bone, might be single byte

	uint16_t unused; // possibly unused

	// general params
	float length; // how far from bone base, along bone, is tip
	float tipMass;

	float unkfloat; // v54 adds an extra value here but otherwise the same
	// observed values are between 0-1

// flexible params
	float yawStiffness;
	float yawDamping;
	float pitchStiffness;
	float pitchDamping;
	float alongStiffness;
	float alongDamping;

	// angle constraint
	float angleLimit; // maximum deflection of tip in radians

	// yaw constraint
	float minYaw; // in radians
	float maxYaw; // in radians
	float yawFriction;
	float yawBounce;

	// pitch constraint
	float minPitch; // in radians
	float maxPitch; // in radians
	float pitchFriction;
	float pitchBounce;

	// base spring
	float baseMass;
	float baseStiffness;
	float baseDamping;
	float baseMinLeft;
	float baseMaxLeft;
	float baseLeftFriction;
	float baseMinUp;
	float baseMaxUp;
	float baseUpFriction;
	float baseMinForward;
	float baseMaxForward;
	float baseForwardFriction;
};

// bone id in alphabetical order
struct mstudiobonenametable_t
{
	uint8_t boneid;
};

struct mstudioattachmentv54_t
{
	uint32_t sznameindex = 0;
	uint32_t flags = 0;

	uint32_t localbone = 0; // parent bone

	matrix3x4_t	localmatrix{}; // attachment point
};

struct FILE_ATTACHMENT
{
	std::string Name = "";

	mstudioattachmentv54_t hdr{};
};

struct FILE_BONE
{
	std::string Name = "";
	std::string SurficeName = "";

	mstudiobonev54_t hdr{};

	uint32_t flag;
	uint32_t parent;
	Vector3 pos;
	Quaternion quat;
	RadianEuler rot;
	matrix3x4_t poseToBone;
};

struct FILE_SEQ
{
	std::string Name = "";
	std::string Activity = "";

	mstudioseqdesc_t hdr{};
};

struct FILE_Out
{
	studiohdr_t hdr{};

	std::vector<FILE_BONE> BoneData{};
	std::vector<FILE_ATTACHMENT> AttachData{};

	std::vector<mstudiobonenametable_t> BoneNameTable{};

	mstudiolinearbonev54_t lhdr{};

	std::vector<FILE_SEQ> seqlist{};

	std::vector<mstudiojigglebonev54_t> JiggleList{};
	std::vector<uint8_t> JiggleDataIndexes{};
	std::vector<uint8_t> JiggleData{};

	inline int FindBoneByName(std::string name)
	{
		if (!name.length())
		{
			printf("Bone '%s' not found in source file\n", name.c_str());
			return -1;
		}

		for (int i = 0; i < BoneData.size(); i++)
		{
			auto Bone = BoneData[i];
			if (Bone.Name == name)
				return i;
		}
	}

	inline FILE_BONE GetBoneDataById(int boneid)
	{
		if (boneid > BoneData.size())
			return {};

		return BoneData[boneid];
	}

	inline std::vector<FILE_BONE> GetBoneParents(int boneid)
	{
		if (boneid > BoneData.size())
			return {};

		FILE_BONE MasterBone = GetBoneDataById(boneid);

		std::vector<FILE_BONE> Bones{};

		uint32_t boneparentid = MasterBone.parent;
		while (boneparentid != -1)
		{
			auto Bone = BoneData[boneparentid];

			Bones.push_back(Bone);

			boneparentid = Bone.parent;
		}

		return Bones;
	}

	inline Vector3 GetPosById(int boneid)
	{
		if (boneid > BoneData.size())
			return {};

		Vector3 Pos = GetBoneDataById(boneid).pos;

		for (auto& bone : GetBoneParents(boneid))
			Pos += bone.pos;

		return Pos;
	}

	// get child bone ids from parent id
	inline std::vector<int> GetChildBonesIds(int parentboneid)
	{
		if (parentboneid > BoneData.size())
			return {};

		std::vector<int> BonesOut{};
		for (int i = 0; i < BoneData.size(); i++)
		{
			auto Bone = BoneData[i];
			if (Bone.parent != -1 && Bone.parent == parentboneid)
				BonesOut.push_back(i);
		}

		return BonesOut;
	}

	inline std::vector<FILE_BONE> GetChildBones(int parentboneid)
	{
		if (parentboneid > BoneData.size())
			return {};

		std::vector<FILE_BONE> BonesOut{};
		std::vector<int> ChildBones = GetChildBonesIds(parentboneid);

		for (int i = 0; i < ChildBones.size(); i++)
		{
			auto& Bone = BoneData[ChildBones[i]];
			if (Bone.parent != -1 && Bone.parent == parentboneid)
				BonesOut.push_back(BoneData[i]);
		}

		return BonesOut;
	}

	// move child bones with parent bone
	inline void MoveChildBones(int parentboneid, Vector3 offset)
	{
		if (parentboneid > BoneData.size())
			return;

		std::vector<FILE_BONE> ChildBones = GetChildBones(parentboneid);

		FILE_BONE& PBone = BoneData[parentboneid];
		PBone.pos += offset;

		for (auto& bone : ChildBones)
			bone.pos += offset;
	}
};