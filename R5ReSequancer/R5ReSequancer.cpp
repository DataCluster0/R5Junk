#include "includes.h"

using namespace std::filesystem;

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

	return animv54.size;
};

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++)
	{
		auto tmpfilepath = std::string((const char*)argv[i]);
		if (tmpfilepath.find(".rseq") == -1 || tmpfilepath.find(".SeqData") != -1)
			continue;

		ASEQ_Out* file = new ASEQ_Out();

		auto filepath = std::filesystem::path(tmpfilepath);

		printf("========================\n");
		printf("%s\n", filepath.filename().string().c_str());

		char* pExtDataBuf = nullptr;
		std::string extdatapath = Utils::ChangeExtension(tmpfilepath, ".SeqData");
		size_t extinputsize = 0;
		BinaryIO extreader;
		if (FILE_EXISTS(extdatapath))
		{
			extinputsize = Utils::GetFileSize(extdatapath);
			extreader.open(extdatapath, BinaryIOMode::Read);

			pExtDataBuf = new char[extinputsize];
			extreader.getReader()->read(pExtDataBuf, extinputsize);
			extreader.seek(0, std::ios_base::_Seekbeg);
		}

		BinaryIO reader;
		reader.open(tmpfilepath, BinaryIOMode::Read);

		size_t inputsize = Utils::GetFileSize(tmpfilepath);
		char* pDataBuf = new char[inputsize];

		reader.getReader()->read(pDataBuf, inputsize);

		reader.seek(0, std::ios_base::_Seekbeg);
		
		auto seqdesc = reader.read<mstudioseqdesc_t>();
		uint64_t seqdescoffset = reader.tell();

		file->hdr = seqdesc;

		int numanims = seqdesc.groupsize.x * seqdesc.groupsize.y;
		int posekeys = seqdesc.groupsize.x + seqdesc.groupsize.y;
		int numbones = (seqdesc.activitymodifierindex - seqdesc.weightlistindex) / 4;

		if ((seqdesc.unkindex - seqdesc.animindexindex) < sizeof(mstudioanimdescv54_t_v121))
			numanims = 0;

		bool IsExternal = false;
		bool IsSegmented = false;
		{
			if (seqdesc.posekeyindex)
			{
				reader.seek(seqdesc.posekeyindex, std::ios_base::_Seekbeg);
				for (int i = 0; i < seqdesc.numevents; i++)
					file->posekeys.push_back(reader.read<float>());
			}

			if (seqdesc.numevents)
			{
				uint32_t str_offset = 0;
				for (int i = 0; i < seqdesc.numevents; i++)
				{
					reader.seek(seqdesc.eventindex + (i * sizeof(mstudioeventv54_t_v122)), std::ios_base::_Seekbeg);
					uint64_t baseoffset = reader.tell();

					auto v10 = reader.read<mstudioeventv54_t_v122>();
					mstudioeventv54_t v7;
					{
						v7.cycle = v10.cycle;
						v7.Event = v10.Event;
						v7.type = v10.type;

						for (int i = 0; i < 256; i++)
							v7.options[i] = v10.options[i];
					}

					reader.seek(baseoffset + v10.szeventindex, std::ios_base::_Seekbeg);
					std::string szeventname = reader.readString();

					int strlen = szeventname.length() + 1;

					v7.szeventindex = v10.szeventindex + (i * 4);

					file->events.push_back({ v7 , szeventname });
					str_offset += strlen;
				}
			}

			if (seqdesc.numautolayers)
			{
				file->autolayers.resize(seqdesc.numautolayers);
				for (int i = 0; i < seqdesc.numautolayers; i++)
					file->autolayers.push_back(reader.read<mstudioautolayerv54_t>());
			}

			if (seqdesc.weightlistindex)
			{
				reader.seek(seqdesc.weightlistindex, std::ios_base::_Seekbeg);
				for (int i = 0; i < numbones; i++)
					file->weightlist.push_back(reader.read<float>());
			}

			if (seqdesc.numactivitymodifiers)
			{
				int struct_size = sizeof(mstudioactivitymodifierv53_t);
				uint64_t str_offset = 0;
				for (int i = 0; i < seqdesc.numactivitymodifiers; i++)
				{
					reader.seek(seqdesc.activitymodifierindex + (i * struct_size), std::ios_base::_Seekbeg);
					uint64_t baseoffset = reader.tell();
			
					auto activitymodifier = reader.read<mstudioactivitymodifierv53_t>();
			
					reader.seek(baseoffset + activitymodifier.sznameindex, std::ios_base::_Seekbeg);
					std::string activityname = reader.readString();
					int strlen = activityname.length() + 1;
			
					uint64_t nameoffset = (seqdesc.numactivitymodifiers * struct_size) + str_offset;
					activitymodifier.sznameindex = nameoffset - (i * struct_size);
					file->activitymodifiers.push_back({ activitymodifier , activityname });
			
					str_offset += strlen;
				}
			}

			if (seqdesc.animindexindex)
			{
				reader.seek(seqdesc.animindexindex, std::ios_base::_Seekbeg);
				for (int i = 0; i < numanims; i++)
					file->blendgroups.push_back(reader.read<int>());
			}

			uint64_t str_offset = 0;
			for (int i = 0; i < numanims; i++)
			{
				ASEQ_Out_animdesc animdata;

				reader.seek(file->blendgroups[i], std::ios_base::_Seekbeg);
				uint64_t animdesc_offset = reader.tell();
				auto animdesc_new = reader.read<mstudioanimdescv54_t_v121>();
				mstudioanimdescv54_t animdesc;

				memcpy(&animdesc, &animdesc_new, offsetof(mstudioanimdescv54_t_v121, unk));

				animdesc.sectionframes = animdesc_new.sectionframes;

				animdata.desc = animdesc;

				{
					reader.seek(animdesc_offset + animdesc.sznameindex, std::ios_base::_Seekbeg);
					std::string szname = reader.readString();

					animdesc.sznameindex = +(sizeof(mstudioanimdescv54_t_v121) - sizeof(mstudioanimdescv54_t));
				}

				if (animdesc.numikrules)
				{
					reader.seek(animdesc_offset + animdesc.ikruleindex, std::ios_base::_Seekbeg);
					for (int i = 0; i < animdesc.numikrules; i++)
					{
						auto ikrule_new = reader.read<mstudioikrulev54_t_new>();

						mstudioikrulev54_t ikrule;

						memcpy(&ikrule, &ikrule_new, offsetof(mstudioikrulev54_t_new, q));

						ikrule.q = ikrule_new.q;
						ikrule.iStart = ikrule_new.iStart;
						ikrule.start = ikrule_new.start;
						ikrule.peak = ikrule_new.peak;
						ikrule.tail = ikrule_new.tail;
						ikrule.end = ikrule_new.end;
						ikrule.contact = ikrule_new.contact;
						ikrule.drop = ikrule_new.drop;
						ikrule.top = ikrule_new.top;
						ikrule.szattachmentindex = ikrule_new.szattachmentindex;

						for (int j = 0; j < 7; j++)
							ikrule.unk1[j] = 0;

						animdata.ikrules.push_back(ikrule);
					}
				}

				if (animdesc.flags & STUDIO_ALLZEROS)
				{
					// there is no actual flag array I just do this so the flag array index will line up with the animdesc.
					std::byte flagarray;
					std::byte ALLZEROSFLAGPLACEHOLDER;
				}
				else
				{
					if (animdesc.sectionindex)
					{
						int sectionlength = ((animdesc.numframes - 1) / animdesc.sectionframes) + 2;

						//FSeek(startof(animdesc[k]) + animdesc[k].sectionindex);
						reader.seek(animdesc_offset + animdesc.sectionindex, std::ios_base::_Seekbeg);
						std::vector<mstudioanimsectionsv53_t> Sections;

						uint64_t animindex_offset = reader.tell();

						for (int i = 0; i < sectionlength; i++)
						{
							auto section = reader.read<mstudioanimsectionsv54_t_v121>();

							if (section.isExternal)
								IsExternal = true;

							animdata.sections.push_back(section);
						}

						reader.seek(animdesc_offset + animdesc.animindex, std::ios_base::_Seekbeg);

						for (int i = 0; i < numbones; i++)
						{
							animflagarrayv54_t boneflag = reader.read<animflagarrayv54_t>();
							animdata.flagarray.push_back(boneflag);
						}

						if (numbones % 4)
							for (int j = 0; j < (4 - (numbones % 4)) / 2; j++)
								reader.read<char>();

						uint64_t oldoffset = reader.tell();

						size_t animdata_size = 0;
						for (int j = 0; j < numbones; j++)
						{
							animflagarrayv54_t flag = animdata.flagarray[j];
							auto startofanimheader = reader.tell();
							animdata_size = +Read_mstudio_rle_anim_t(reader, flag);

							reader.seek(startofanimheader, std::ios_base::_Seekbeg);
						}
					}
					else
					{
						reader.seek(animdesc_offset + animdesc.animindex, std::ios_base::_Seekbeg);

						for (int i = 0; i < numbones; i++)
						{
							animflagarrayv54_t boneflag = reader.read<animflagarrayv54_t>();
							animdata.flagarray.push_back(boneflag);
						}

						if (numbones % 4)
							for (int j = 0; j < (4 - (numbones % 4)) / 2; j++)
								reader.read<char>();

						uint64_t oldoffset = reader.tell();

						size_t animdata_size = 0;
						for (int j = 0; j < numbones; j++)
						{
							animflagarrayv54_t flag = animdata.flagarray[j];
							auto startofanimheader = reader.tell();
							animdata_size = +Read_mstudio_rle_anim_t(reader, flag);

							reader.seek(startofanimheader, std::ios_base::_Seekbeg);
						}
					}
				}

				file->animdescs.push_back(animdata);
			}

			reader.seek(seqdesc.szlabelindex, std::ios_base::_Seekbeg);
			file->szlabel = reader.readString();

			reader.seek(seqdesc.szactivitynameindex, std::ios_base::_Seekbeg);
			file->szactivity = reader.readString();

			reader.close();

			if (IsExternal)
				printf("%s -> external data detected...\n", filepath.filename().string().c_str());

			BinaryIO writer;
			std::string outpath = filepath.parent_path().string() + "\\downgraded\\";

			std::filesystem::create_directory(outpath);
			writer.open(outpath + filepath.filename().string(), BinaryIOMode::Write);

			writer.getWriter()->write(pDataBuf, inputsize);
			writer.seek(inputsize, std::ios_base::_Seekbeg);

			if (IsExternal)
				writer.getWriter()->write(pExtDataBuf, extinputsize);

			if (IsExternal && !pExtDataBuf)
			{
				printf("%s -> .seqdata not found skipping...\n", filepath.filename().string().c_str());
				continue;
			}

			if (file->events.size())
			{
				writer.seek(file->hdr.eventindex, std::ios_base::_Seekbeg);
				file->hdr.eventindex = writer.tell();
				for (int i = 0; i < file->events.size(); i++)
					writer.write<mstudioeventv54_t>(file->events[i].event);
			}

			if (file->animdescs.size())
			{
				uint64_t animdatasize = 0;
				for (int i = 0; i < file->animdescs.size(); i++)
				{
					auto offset = file->blendgroups[i];
					auto animdesc = file->animdescs[i];

					writer.seek(offset + animdesc.desc.ikruleindex, std::ios_base::_Seekbeg);
					for (int i = 0; i < animdesc.ikrules.size(); i++)
						writer.write<mstudioikrulev54_t>(animdesc.ikrules[i]);

					if (animdesc.sections.size())
					{
						writer.seek(offset + animdesc.desc.sectionindex, std::ios_base::_Seekbeg);

						bool HasExternal = false;

						int realsize = animdesc.sections.size() - 2;

						for (int i = 1; i < animdesc.sections.size(); i++)
						{
							auto section = animdesc.sections[i];
							auto oldpos = writer.tell();

							if (section.isExternal)
								section.animindex = (inputsize + section.animindex) - offset;

							if (section.isExternal)
								HasExternal = true;

							writer.write<int>(section.animindex);
						}

						// remove null section
						if (HasExternal)
							animdesc.desc.numframes = animdesc.desc.sectionframes * (animdesc.sections.size() - 3);
					}

					writer.seek(offset, std::ios_base::_Seekbeg);
					writer.write<mstudioanimdescv54_t>(animdesc.desc);
				}
			}

			writer.seek(0, std::ios_base::_Seekend);
			uint64_t offset = writer.tell();
			writer.writeString(file->szlabel);

			file->hdr.szlabelindex = offset;

			//// write new header data
			writer.seek(0, std::ios_base::_Seekbeg);
			writer.write<mstudioseqdesc_t>(file->hdr);

			writer.close();
		}
	}

	system("pause");
	return EXIT_SUCCESS;
}