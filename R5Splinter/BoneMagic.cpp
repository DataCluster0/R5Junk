#include "BoneMagic.h"

FILE_Out STUDIOMDLReadBones(BinaryIO& Reader, std::string filename)
{
	FILE_Out DataOut{};

	studiohdr_t mdlhdr = Reader.read<studiohdr_t>();

	if (mdlhdr.id != 0x54534449) // "IDST"
	{
		printf("invalid file magic for arig '%s'. expected %x, found %x\n", filename.c_str(), 0x54534449, mdlhdr.id);
		return{};
	}

	if (mdlhdr.version != 54)
	{
		printf("invalid version for arig asset '%s'. expected %i, found %i\n", filename.c_str(), 54, mdlhdr.version);
		return{};
	}

	if (mdlhdr.numbones < 0)
		return{};

	std::vector<FILE_BONE> BoneData{};
	std::vector<FILE_ATTACHMENT> AttachData{};

	DataOut.BoneNameTable.resize(mdlhdr.numbones);
	BoneData.resize(mdlhdr.numbones);
	AttachData.resize(mdlhdr.numlocalattachments);

	// read mstudiobones
	Reader.seek(mdlhdr.boneindex);

	// read attachments
	if (mdlhdr.localattachmentindex)
	{
		for (int i = 0; i < AttachData.size(); i++)
		{
			auto& attachment = AttachData[i];

			uint64_t pos = mdlhdr.localattachmentindex + (i * sizeof(mstudioattachmentv54_t));
			Reader.seek(pos);

			mstudioattachmentv54_t hdr = Reader.read<mstudioattachmentv54_t>();
			attachment.hdr = hdr;

			uint64_t stringpos = pos + hdr.sznameindex;

			Reader.seek(stringpos);
			attachment.Name = Reader.readString();
		}
	}

	// read bones
	if (mdlhdr.boneindex)
	{
		for (int i = 0; i < BoneData.size(); i++)
		{
			auto& bone = BoneData[i];

			uint64_t pos = mdlhdr.boneindex + (i * sizeof(mstudiobonev54_t));
			Reader.seek(pos);

			mstudiobonev54_t hdr = Reader.read<mstudiobonev54_t>();
			bone.hdr = hdr;

			uint64_t stringpos = pos + hdr.sznameindex;

			Reader.seek(stringpos);
			bone.Name = Reader.readString();

			uint64_t surficepos = pos + hdr.surfacepropidx;

			Reader.seek(surficepos);
			bone.SurficeName = Reader.readString();
		}
	}

	// read nametable
	Reader.seek(mdlhdr.bonetablebynameindex);
	for (auto& bone : DataOut.BoneNameTable)
		bone = Reader.read<mstudiobonenametable_t>();

	if (mdlhdr.linearboneindex)
	{
		Reader.seek(mdlhdr.linearboneindex);
		mstudiolinearbonev54_t lbonehdr = Reader.read<mstudiolinearbonev54_t>();
		uint64_t lbonehdroffset = 0;

		// read linear data
		for (auto& bone : BoneData)
			bone.flag = Reader.read<uint32_t>();

		for (auto& bone : BoneData)
			bone.parent = Reader.read<uint32_t>();

		for (auto& bone : BoneData)
			bone.pos = Reader.read<Vector3>();

		for (auto& bone : BoneData)
			bone.quat = Reader.read<Quaternion>();

		for (auto& bone : BoneData)
			bone.rot = Reader.read<RadianEuler>();

		for (auto& bone : BoneData)
			bone.poseToBone = Reader.read<matrix3x4_t>();
	}

	Reader.close();

	DataOut.hdr = mdlhdr;
	DataOut.BoneData = BoneData;
	DataOut.AttachData = AttachData;
	return DataOut;
}

void STUDIOMDLWriteBones(BinaryIO& writer, FILE_Out& Source, FILE_Out& Target, bool linear)
{
	auto& bonedata = Source.BoneData;
	auto& attachdata = Source.AttachData;
	studiohdr_t hdr = Target.hdr;

	// go to file end
	writer.seek(0, std::ios_base::_Seekend);

	if (bonedata.size())
	{
		// write bone headers
		hdr.boneindex = writer.tell();
		uint64_t nameoffset = 0;
		for (int i = 0; i < bonedata.size(); i++)
		{
			auto& bone = bonedata[i];

			uint64_t pos = hdr.boneindex + (i * sizeof(mstudiobonev54_t));
			writer.seek(pos);

			uint64_t stringoffset = hdr.boneindex + (bonedata.size() * sizeof(mstudiobonev54_t)) + nameoffset;
			
			bone.hdr.sznameindex = stringoffset - pos;
			bone.hdr.surfacepropidx = (stringoffset + bone.Name.length() + 1) - pos;

			bone.hdr.flags = bone.flag;
			bone.hdr.parent = bone.parent;
			bone.hdr.pos = bone.pos;
			bone.hdr.quat = bone.quat;
			bone.hdr.rot = bone.rot;
			bone.hdr.poseToBone = bone.poseToBone;


			bone.hdr.proctype = 0;

			writer.write<mstudiobonev54_t>(bone.hdr);

			writer.seek(stringoffset);
			writer.writeString(bone.Name);

			writer.writeString(bone.SurficeName);

			
			nameoffset += (bone.Name.length() + 1) + (bone.SurficeName.length() + 1) + 164;
		}

		hdr.localattachmentindex = writer.tell();
		for (int i = 0; i < attachdata.size(); i++)
		{
			auto& attachment = attachdata[i];

			uint64_t pos = hdr.localattachmentindex + (i * sizeof(mstudioattachmentv54_t));
			writer.seek(pos);

			uint64_t stringoffset = hdr.localattachmentindex + (bonedata.size() * sizeof(mstudioattachmentv54_t)) + nameoffset;

			attachment.hdr.sznameindex = stringoffset - pos;

			writer.write<mstudioattachmentv54_t>(attachment.hdr);

			writer.seek(stringoffset);

			writer.writeString(attachment.Name);

			nameoffset += attachment.Name.length() + 1;
		}


		hdr.bonetablebynameindex = writer.tell();

		for (auto& name : Source.BoneNameTable)
			writer.write<mstudiobonenametable_t>(name);

		if (linear)
		{
			hdr.linearboneindex = writer.tell();

			mstudiolinearbonev54_t lbonetable{};
			lbonetable.numbones = bonedata.size();

			writer.write<mstudiolinearbonev54_t>(lbonetable);

			// write linear data

			lbonetable.flagsindex = writer.tell() - hdr.linearboneindex;
			for (auto& bone : bonedata)
				writer.write<uint32_t>(bone.flag);

			lbonetable.parentindex = writer.tell() - hdr.linearboneindex;
			for (auto& bone : bonedata)
				writer.write<uint32_t>(bone.parent);

			lbonetable.posindex = writer.tell() - hdr.linearboneindex;
			for (auto& bone : bonedata)
				writer.write<Vector3>(bone.pos);

			lbonetable.quatindex = writer.tell() - hdr.linearboneindex;
			for (auto& bone : bonedata)
				writer.write<Quaternion>(bone.quat);

			lbonetable.rotindex = writer.tell() - hdr.linearboneindex;
			for (auto& bone : bonedata)
				writer.write<RadianEuler>(bone.rot);

			lbonetable.posetoboneindex = writer.tell() - hdr.linearboneindex;
			for (auto& bone : bonedata)
				writer.write<matrix3x4_t>(bone.poseToBone);

			// write new lbone header data
			writer.seek(hdr.linearboneindex);
			writer.write<mstudiolinearbonev54_t>(lbonetable);
		}
	}

	hdr.numlocalseq = 0;
	hdr.numbones = bonedata.size();
	hdr.length = writer.size();
	// write studio header
	writer.seek(0);
	writer.write<studiohdr_t>(hdr);
	writer.close();
};