#pragma once
#include <includes.h>
using namespace std;

#define VG_POSITION         0x1
#define VG_PACKED_POSITION  0x2
#define VG_VERTEX_COLOR     0x10
#define VG_PACKED_WEIGHTS   0x5000
#define VG_UV_LAYER         0x200000000 // what

#define MAX_NUM_BONES_PER_VERT 3

const float onekb = 0.0009765625;

enum StripHeaderFlags_t : uint8_t
{
	STRIP_IS_TRILIST = 0x01,
	STRIP_IS_QUADLIST_REG = 0x02,		// Regular sub-d quads
	STRIP_IS_QUADLIST_EXTRA = 0x04		// Extraordinary sub-d quads
};

struct PackedPosition
{
	union {
		uint64_t value;

		union
		{
			DWORD dword[2];
		} d;
	};

	inline Vector3 UnpackPosition()
	{
		float x = ((d.dword[0] & 0x1FFFFF) * onekb) - 1024.0;
		float y = ((((d.dword[1] & 0x3FF) << 11) + (d.dword[0] >> 21)) * onekb) - 1024.0;
		float z = ((d.dword[1] >> 10) * onekb) - 2048.0;

		return Vector3(x, y, z);
	};

	static inline PackedPosition packPosition(Vector3 pos)
	{
		PackedPosition Data{};

		// x
		Data.d.dword[0] = (static_cast<uint32_t>((pos.X + 1024.0) / onekb)) ^ 2;

		// y
		Data.d.dword[0] += (static_cast<uint32_t>((pos.Y / onekb)) << 21);
		Data.d.dword[1] = (static_cast<uint32_t>(((pos.Y + 1024.0) / onekb)) >> 11);

		// z
		Data.d.dword[1] += (static_cast<uint32_t>(((pos.Z + 2048.0) / onekb)) << 10);
		return Data;
	}

	inline void MovePosition(Vector3 pos)
	{
		Vector3 o = UnpackPosition();

		value = packPosition(o + pos).value;
	}
};




struct PackedVertexWeights
{
	short BlendWeights[2]; // haha jk.. unless?
	byte BlendIds[4]; // last two flags?
};

struct extendedWeight
{
	byte unk[4];
};

struct mstudioboneweight_t
{
	float	weight[MAX_NUM_BONES_PER_VERT];
	char	bone[MAX_NUM_BONES_PER_VERT];
	uint8_t	numbones = 0;
};

struct VertexColor_t
{
	uint8_t r, g, b, a;
};

struct unkdata
{
	int64_t unk;
	float unk1;

	char data[0x24];
};

struct RMdlVGIndexCountPacked
{
	uint64_t Count : 56;
	uint64_t Type : 8;
};

// A strip is a piece of a stripgroup which is divided by bones
struct VGStrip
{
	int numIndices = 0;
	int indexOffset = 0;

	int numVerts = 0;
	int vertOffset = 0;

	short numBones = 0;

	StripHeaderFlags_t flags = StripHeaderFlags_t::STRIP_IS_TRILIST;

	int numBoneStateChanges = 0;
	int boneStateChangeOffset = 0;

	// MDL Version 49 and up only
	int numTopologyIndices = 0;
	int topologyOffset = 0;
};

struct VGMeshNEW
{
	__int64 flags = 0;

	int vertCacheSize = 0;		        // number of bytes used from the vertex buffer
	int vertexCount = 0;			    // number of vertices

	__int64 indexOffset = 0;            // start offset for this mesh's "indices"
	RMdlVGIndexCountPacked indexPacked{};	// 0x2 each (uint16_t)

	__int64 vertexOffset = 0;           // start offset for this mesh's vertices
	__int64 vertexBufferSize = 0;       // TOTAL size of vertex buffer

	__int64 extendedWeightsOffset = 0;	// start offset for this mesh's "extended weights"
	__int64 extendedWeightsSize = 0;    // size or count of extended weights

	__int64 externalWeightsOffset = 0;	// seems to be an offset into the "external weights" buffer for this mesh
	__int64 externalWeightsCount = 0;   // seems to be the number of "external weights" that this mesh uses

	__int64 stripsOffset = 0;			// Index into the strips structs
	__int64 stripsCount = 0;
};

struct VGMesh
{
	__int64 flags = 0;	// mesh flags

	int vertexOffset = 0;			// start offset for this mesh's vertices
	int vertCacheSize = 0;		    // number of bytes used from the vertex buffer
	int vertexCount = 0;			// number of vertices

	int unk1 = 0;

	int extendedWeightsOffset = 0;	// start offset for this mesh's "extended weights"
	int extendedWeightsSize = 0;    // size or count of extended weights

	int indexOffset = 0;			// start offset for this mesh's "indices"
	int indexCount = 0;				// number of indices

	int externalWeightsOffset = 0;	// seems to be an offset into the "external weights" buffer for this mesh
	int externalWeightsCount = 0;   // seems to be the number of "external weights" that this mesh uses

	int stripsOffset = 0;			// Index into the strips structs
	int stripsCount = 0;

	int Int15 = 0;
	int Int16 = 0;
	int Int17 = 0;
	int Int18 = 0;
};

struct VGLodNEW
{
	int dataOffset = 0; // stolen from rmdl
	uint32_t dataSize = 0;

	// this is like the section in rmdl, but backwards for some reason
	uint8_t meshCount = 0;
	uint8_t meshIndexOffset = 0; // for lod, probably 0 in most cases
	uint8_t unklodlevel = 0;
	uint8_t unklodlevel1 = 0;

	float switchPoint = 0.0;

	__int64 meshOffset = 0;
};

struct VGLod
{
	uint16_t meshIndex = 0;
	uint16_t meshCount = 0;
	float distance = 0.0;
};

struct VGHeaderNEW
{
	uint32_t magic = '0tVG';	// 0x47567430	'0tVG'
	uint32_t version = 0;	    // 0x1
	uint32_t lodLevel = 0;    // from RMDL struct
	uint32_t lodCount = 0;	// If 0x1, this IS the first and only lod, if > 0x1, MORE 0tVG headers follow PER lod count
	uint32_t unk = 0;
	uint32_t unk1 = 0;        // unk2 in RMDL struct
	uint32_t lodOffset = 0;

	// v14 unks
	char unk3[8];
};

struct VGHeader
{
	uint32_t magic = '0tvg';		// 0x47567430	'0tvg'
	uint32_t version = 0x1;	    // 0x1
	uint32_t unk = 0;	        // Usually 0
	uint32_t dataSize = 0;	// Total size of data + header in starpak

	uint64_t boneRemapOffset = 0; // offset to bone remap buffer
	uint64_t boneRemapCount = 0;  // number of "bone remaps" (size: 1)

	uint64_t meshOffset = 0;   // offset to mesh buffer
	uint64_t meshCount = 0;    // number of meshes (size: 0x48)

	uint64_t indexOffset = 0;     // offset to index buffer
	uint64_t indexCount = 0;      // number of indices (size: 2 (uint16_t))

	uint64_t vertexOffset = 0;    // offset to vertex buffer
	uint64_t vertexCount = 0;     // number of bytes in vertex buffer

	uint64_t extendedWeightsOffset = 0;   // offset to extended weights buffer
	uint64_t extendedWeightsCount = 0;    // number of bytes in extended weights buffer

	// there is one for every LOD mesh
	// i.e, unknownCount == lod.meshCount for all LODs
	uint64_t unknownOffset = 0;   // offset to buffer
	uint64_t unknownCount = 0;    // count (size: 0x30)

	uint64_t lodOffset = 0;       // offset to LOD buffer
	uint64_t lodCount = 0;        // number of LODs (size: 0x8)

	uint64_t externalWeightsOffset = 0;   // offset to external weights buffer
	uint64_t externalWeightsCount = 0;      // number of external weights (size: 0x10)

	uint64_t stripsOffset = 0;    // offset to strips buffer
	uint64_t stripsCount = 0;     // number of strips (size: 0x23)

	char unused[0x40];

	inline void FromNewHeader(VGHeaderNEW nh)
	{
		magic = nh.magic;
		version = nh.version;
		lodCount = nh.lodCount;
		lodOffset = nh.lodOffset;
	}

	inline void FromNewMesh(VGMeshNEW nh)
	{
		extendedWeightsOffset = nh.externalWeightsOffset;
		extendedWeightsCount = nh.externalWeightsCount;
		vertexOffset = nh.vertexOffset;
		vertexCount = nh.vertexCount;
		stripsOffset = nh.stripsOffset;
		stripsCount = nh.stripsCount;
	}
};

struct VTX
{
	Vector3 NewPos{};

	Vector3 Pos{};
	PackedPosition PKPos{};

	PackedVertexWeights PKWeights{};

	DWORD normal{}; // packed normal

	// vertex color from vvc
	VertexColor_t color{};

	Vector2 TexCoord{};

	// same vector2 from vvc
	Vector2 UVLayer{};


	static inline VTX ReadVTX(BinaryIO& Reader, VGMeshNEW& Mesh)
	{
		VTX o{};

		if (Mesh.flags & VG_POSITION)
			o.Pos = Reader.read<Vector3>();
		else if (Mesh.flags & VG_PACKED_POSITION)
			o.PKPos = Reader.read<PackedPosition>();

		if (Mesh.flags & VG_PACKED_WEIGHTS)
			o.PKWeights = Reader.read<PackedVertexWeights>();

		o.normal = Reader.read<DWORD>(); // packed normal

		// vertex color from vvc
		if (Mesh.flags & VG_VERTEX_COLOR)
			o.color = Reader.read<VertexColor_t>();

		o.TexCoord = Reader.read<Vector2>();

		// same vector2 from vvc
		if (Mesh.flags & VG_UV_LAYER)
			o.UVLayer = Reader.read<Vector2>();

		return o;
	};

	static inline VTX ReadVTX(BinaryIO& Reader, VGMesh& Mesh)
	{
		VTX o{};

		if (Mesh.flags & VG_POSITION)
			o.Pos = Reader.read<Vector3>();
		else if (Mesh.flags & VG_PACKED_POSITION)
			o.PKPos = Reader.read<PackedPosition>();

		if (Mesh.flags & VG_PACKED_WEIGHTS)
			o.PKWeights = Reader.read<PackedVertexWeights>();

		o.normal = Reader.read<DWORD>(); // packed normal

		// vertex color from vvc
		if (Mesh.flags & VG_VERTEX_COLOR)
			o.color = Reader.read<VertexColor_t>();

		o.TexCoord = Reader.read<Vector2>();

		// same vector2 from vvc
		if (Mesh.flags & VG_UV_LAYER)
			o.UVLayer = Reader.read<Vector2>();

		return o;
	};
};


