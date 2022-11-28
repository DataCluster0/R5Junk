
#include <includes.h>
#include "SEAnim.h"

int GetBoneTSize(int bonecount)
{
	if (bonecount <= 0xFF)
		return sizeof(uint8_t);
	else if (bonecount <= 0xFFFF)
		return sizeof(uint16_t);

	return sizeof(uint32_t);
}

uint32_t ReadBoneT(BinaryIO& reader, int bonecount)
{
	if (bonecount <= 0xFF)
		return reader.read<uint8_t>();
	else if (bonecount <= 0xFFFF)
		return reader.read<uint16_t>();

	return reader.read<uint32_t>();
}

std::vector<SEAnimBone_o> ReadSEAnim(BinaryIO& reader)
{
	SEAnim_File_Header_t ih = reader.read<SEAnim_File_Header_t>();

	if (std::string(ih.magic).compare("SEAnim") == -1)
	{
		printf("invalid file magic expected '%s', found '%s'\n", SEANIM_MAGIC, std::string(ih.magic).c_str());
		return {};
	}

	if (ih.version != SEANIM_VERSION)
	{
		printf("invalid file version expected '%d', found '%d'\n", SEANIM_VERSION, ih.version);
		return {};
	}

	auto& h = ih.header;

	// read bone name
	std::vector<std::string> BoneNames(h.boneCount);
	for (auto& name : BoneNames)
		name = reader.readString();

	//
	int numSize = GetBoneTSize(h.boneCount); // determine bone index length in bytes depending on bone count
	std::vector<SEAnim_BoneAnimModifier_t> BoneModifiers(h.boneAnimModifierCount);
	for (auto& mod : BoneModifiers)
	{
		mod.index = ReadBoneT(reader, h.boneCount);
		mod.animTypeOverride = reader.read<SEAnim_AnimationType>();
	}

	numSize = GetBoneTSize(h.frameCount); // determine frame index length in bytes depending on frame count
	std::vector<SEAnim_BoneData_t> BoneData(h.boneCount);
	for (auto& bone : BoneData)
	{
		bone.flags = reader.read<uint8_t>();

		if (h.dataPresenceFlags & SEANIM_BONE_LOC)
		{
			bone.poslist.resize(ReadBoneT(reader, h.boneCount));
			for (auto& pos : bone.poslist)
			{
				pos.frame = ReadBoneT(reader, h.boneCount);
				pos.pos = reader.read<Vector3>();
			}
		}

		if (h.dataPresenceFlags & SEANIM_BONE_ROT)
		{
			bone.rotlist.resize(ReadBoneT(reader, h.boneCount));
			for (auto& rot : bone.rotlist)
			{
				rot.frame = ReadBoneT(reader, h.boneCount);
				rot.rot = reader.read<Quaternion>();
			}
		}

		if (h.dataPresenceFlags & SEANIM_BONE_SCALE)
		{
			bone.scalelist.resize(ReadBoneT(reader, h.boneCount));
			for (auto& scale : bone.scalelist)
			{
				scale.frame = ReadBoneT(reader, h.boneCount);
				scale.scale = reader.read<Vector3>();
			}
		}
	}

	std::vector<SEAnimBone_o> BonesOut(h.boneCount);
	for (int i = 0; i < BonesOut.size(); i++)
	{
		auto& bone = BonesOut[i];

		bone.Name = BoneNames[i];

		auto& framedata = BoneData[i];

		bone.flags = framedata.flags;

		std::vector<SEAnimBoneFrame_o> Frames(h.frameCount);

		if (framedata.poslist.size())
		{
			for (int i = 0; i < framedata.poslist.size(); i++)
			{
				auto& frame = framedata.poslist[i];

 				Frames[frame.frame].pos = frame.pos;
			}
		}

		if (framedata.rotlist.size())
		{
			for (int i = 0; i < framedata.rotlist.size(); i++)
			{
				auto& frame = framedata.rotlist[i];

				Frames[frame.frame].rot = frame.rot;
			}
		}

		if (framedata.scalelist.size())
		{
			for (int i = 0; i < framedata.scalelist.size(); i++)
			{
				auto& frame = framedata.scalelist[i];

				Frames[frame.frame].scale = frame.scale;
			}
		}

		bone.Frames = Frames;
	}

	reader.close();

	return BonesOut;
}