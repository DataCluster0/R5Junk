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

		int jigglebonecount = 0;

		for (int i = 0; i < BoneData.size(); i++)
		{
			auto& bone = BoneData[i];

			if (bone.hdr.proctype == 5)
				jigglebonecount++;
		}

		if (jigglebonecount)
		{
			Reader.seek(mdlhdr.boneindex + (BoneData.size() * sizeof(mstudiobonev54_t)));

			for (int i = 0; i < jigglebonecount; i++)
				DataOut.JiggleList.push_back(Reader.read<mstudiojigglebonev54_t>());

			Reader.seek(mdlhdr.m_nBoneFlexDriverIndex);

			for (int i = 0; i < mdlhdr.m_nBoneFlexDriverCount; i++)
				DataOut.JiggleDataIndexes.push_back(Reader.read<uint8_t>());

			Reader.seek(mdlhdr.unkindexflex);
			for (int i = 0; i < BoneData.size(); i++)
				DataOut.JiggleData.push_back(Reader.read<uint8_t>());
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

	if (mdlhdr.localseqindex && mdlhdr.numlocalseq)
	{
		for (int i = 0; i < mdlhdr.numlocalseq; i++)
		{
			FILE_SEQ file{};
			Reader.seek(mdlhdr.localseqindex + (i * sizeof(mstudioseqdesc_t)));
			uint64_t seqpos = Reader.tell();

			file.hdr = Reader.read<mstudioseqdesc_t>();

			Reader.seek(seqpos + file.hdr.szlabelindex);
			file.Name = Reader.readString();

			Reader.seek(seqpos + file.hdr.szactivitynameindex);
			file.Activity = Reader.readString();

			DataOut.seqlist.push_back(file);
		}
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

			bone.hdr.flags = bone.flag;
			bone.hdr.parent = bone.parent;
			bone.hdr.pos = bone.pos;
			bone.hdr.quat = bone.quat;
			bone.hdr.rot = bone.rot;
			bone.hdr.poseToBone = bone.poseToBone;

			writer.write<mstudiobonev54_t>(bone.hdr);
		}

		if (Source.JiggleList.size())
		{
			writer.seek(hdr.boneindex + (bonedata.size() * sizeof(mstudiobonev54_t)));

			for (auto& JiggleBone : Source.JiggleList)
				writer.write<mstudiojigglebonev54_t>(JiggleBone);

			hdr.m_nBoneFlexDriverIndex = writer.tell();
			hdr.m_nBoneFlexDriverCount = Source.JiggleDataIndexes.size();

			for (auto& Index : Source.JiggleDataIndexes)
				writer.write<uint8_t>(Index);

			hdr.unkindexflex = writer.tell();

			for (auto& Data : Source.JiggleData)
				writer.write<uint8_t>(Data);
		}

		// write bone strings
		for (int i = 0; i < bonedata.size(); i++)
		{
			auto& bone = bonedata[i];

			uint64_t pos = hdr.boneindex + (i * sizeof(mstudiobonev54_t));
			writer.seek(pos);

			uint64_t size = (bonedata.size() * sizeof(mstudiobonev54_t)) + (Source.JiggleList.size() * sizeof(mstudiojigglebonev54_t)) + 
				            (Source.JiggleDataIndexes.size() * sizeof(uint8_t)) + (Source.JiggleData.size() * sizeof(uint8_t));

			uint64_t stringoffset = hdr.boneindex + size + nameoffset;

			bone.hdr.sznameindex = stringoffset - pos;
			bone.hdr.surfacepropidx = (stringoffset + bone.Name.length() + 1) - pos;

			writer.write<mstudiobonev54_t>(bone.hdr);

			writer.seek(stringoffset);

			writer.writeString(bone.Name);
			writer.writeString(bone.SurficeName);

			nameoffset += (bone.Name.length() + 1) + (bone.SurficeName.length() + 1);
		}

		if (attachdata.size() > 0)
		{
			hdr.localattachmentindex = writer.tell();
			hdr.numlocalattachments = attachdata.size();
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

		//if (Source.seqlist.size())
		//{
		//	hdr.numlocalseq = Source.seqlist.size();
		//
		//	for (int i = 0; i < Source.seqlist.size(); i++)
		//	{
		//		FILE_SEQ& file = Source.seqlist[i];
		//
		//		uint64_t weightpos = writer.tell();
		//		{
		//			float weight = 1;
		//
		//			for (auto& bone : bonedata)
		//				writer.write<float>(weight);
		//		}
		//
		//		writer.writeString(file.Name);
		//		writer.writeString(file.Activity);
		//
		//		writer.seek(hdr.localseqindex + (i * sizeof(mstudioseqdesc_t)));
		//		file.hdr.weightlistindex = weightpos - writer.tell();
		//		file.hdr.szlabelindex = (weightpos + (bonedata.size() * 4)) - writer.tell();
		//		file.hdr.szactivitynameindex = file.hdr.szlabelindex + file.Name.length() + 1;
		//
		//		writer.write<mstudioseqdesc_t>(file.hdr);
		//	}
		//}
	}

	//hdr.numlocalseq = 0;
	hdr.numbones = bonedata.size();
	hdr.length = writer.size();
	// write studio header
	writer.seek(0);
	writer.write<studiohdr_t>(hdr);
	writer.close();
};