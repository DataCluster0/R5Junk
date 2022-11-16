#include "VGMagic.h"

VGFileNew ReadNewVG(BinaryIO& Reader)
{
	VGFileNew file{};

	file.h = Reader.read<VGHeaderNEW>();
	uint64_t hpos = Reader.tell();

	VGHeaderNEW& h = file.h;

	std::vector<VGFileLod>& Lods = file.Lods;

	Lods.resize(h.lodCount);

	for (int i = 0; i < Lods.size(); i++)
	{
		auto& data = Lods[i];

		Reader.seek(hpos + h.lodOffset + (sizeof(VGLodNEW) * i) + h.lodOffset);
		data.lod = Reader.read<VGLodNEW>();
		auto& lod = data.lod;

		Reader.seek(hpos + h.lodOffset + data.lod.meshOffset - 64);
		data.hdr = Reader.read<VGHeaderNEW>();
		uint64_t hpos2 = Reader.tell();

		auto& hdr = data.hdr;

		Reader.seek(hpos2 + hdr.lodOffset + hdr.lodOffset);
		data.vglod = Reader.read<VGLodNEW>();
		uint64_t lpos = Reader.tell();

		auto& vglod = data.vglod;

		for (int j = 0; j < lod.meshCount; ++j)
		{
			Reader.seek(lpos + lod.meshOffset + lod.meshOffset + (j * sizeof(VGMeshNEW)));
			data.mesh = Reader.read<VGMeshNEW>();
			uint64_t mpos = Reader.tell();

			auto& mesh = data.mesh;

			if (mesh.flags == 0 || mesh.vertexCount == 0)
				continue;

			// Indices

			Reader.seek(mpos + mesh.indexOffset + mesh.indexOffset);

			// read index data
			for (int o = 0; o < mesh.indexPacked.Count; o++)
				data.indexdata.push_back(Reader.read<uint16_t>());

			// Vertices

			Reader.seek(mpos + mesh.vertexOffset + mesh.vertexOffset);

			for (int o = 0; o < mesh.vertexCount; o++)
				data.vtxlist.push_back(Reader.read<VTX>());

			if (mesh.extendedWeightsSize)
			{
				Reader.seek(mesh.extendedWeightsOffset + mesh.extendedWeightsOffset);

				for (int o = 0; o < mesh.extendedWeightsSize / 4; o++)
					data.extweights.push_back(Reader.read<extendedWeight>());
			}

			// External Weights
			if (mesh.externalWeightsCount)
			{
				Reader.seek(mesh.externalWeightsOffset + mesh.externalWeightsOffset);

				for (int o = 0; o < mesh.externalWeightsCount; o++)
					data.boneweigths.push_back(Reader.read<mstudioboneweight_t>());
			}

			Reader.seek(mesh.stripsOffset + mesh.stripsOffset);
			data.stp = Reader.read<VGStrip>();
		}
	}

	return file;
}