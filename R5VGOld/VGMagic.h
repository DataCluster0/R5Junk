#pragma once
#include <includes.h>

struct VGFileLod
{
	VGLodNEW lod;
	VGMeshNEW mesh;

	VGHeaderNEW hdr;

	VGLodNEW vglod;

	std::vector<uint16_t> indexdata;
	std::vector<VTX> vtxlist;
	std::vector<extendedWeight> extweights;
	std::vector<mstudioboneweight_t> boneweigths;

	VGStrip stp;
};

struct VGFileNew
{
	VGHeaderNEW h;

	std::vector<VGFileLod> Lods;
};

struct VGFile
{
	VGHeader Hdr;
};

VGFileNew ReadNewVG(BinaryIO& Reader);
