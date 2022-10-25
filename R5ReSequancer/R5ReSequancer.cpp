#include "includes.h"
#include "BinaryIO.h"
#include "Utils.h"
#include "structs.h"

using namespace std::filesystem;

int main(int argc, char* argv[]) {

	for (int i = 0; i < argc; i++)
	{
		auto tmpfilepath = std::string((const char*)argv[i]);
		if (tmpfilepath.find(".rseq") == -1)
			continue;
		ASEQ_Out* file = new ASEQ_Out();

		auto filepath = std::filesystem::path(tmpfilepath);

		printf("========================\n");
		printf("%s\n", filepath.filename().string().c_str());

		BinaryIO reader;
		reader.open(tmpfilepath, BinaryIOMode::Read);

		size_t inputsize = Utils::GetFileSize(tmpfilepath);
		char* pDataBuf = new char[inputsize];

		reader.getReader()->read(pDataBuf, inputsize);

		reader.seek(0, std::ios_base::_Seekbeg);
		auto seqdesc = reader.read<mstudioseqdesc_t>();
		uint64_t seqdescoffset = reader.getPosition();

		file->hdr = seqdesc;

		int numanims = seqdesc.groupsize[0] * seqdesc.groupsize[1];
		int posekeys = seqdesc.groupsize[0] + seqdesc.groupsize[1];
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
					uint64_t baseoffset = reader.getPosition();

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
					uint64_t baseoffset = reader.getPosition();

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
				uint64_t animdesc_offset = reader.getPosition();
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

						uint64_t animindex_offset = reader.getPosition();

						for (int i = 0; i < sectionlength; i++)
						{
							auto section = reader.read<mstudioanimsectionsv54_t_v121>();

							if (section.isExternal)
							{
								IsExternal = true;
								continue;
							}

							animdata.sections.push_back({ section.animindex });
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

						uint64_t oldoffset = reader.getPosition();

						size_t animdata_size = 0;
						for (int j = 0; j < numbones; j++)
						{
							animflagarrayv54_t flag = animdata.flagarray[j];
							auto startofanimheader = reader.getPosition();
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

						uint64_t oldoffset = reader.getPosition();

						size_t animdata_size = 0;
						for (int j = 0; j < numbones; j++)
						{
							animflagarrayv54_t flag = animdata.flagarray[j];
							auto startofanimheader = reader.getPosition();
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

			if(IsSegmented)
				printf("%s -> segmented data detected...\n", filepath.filename().string().c_str());

			BinaryIO writer;
			std::string outpath = filepath.parent_path().string() + "\\downgraded\\";

			std::filesystem::create_directory(outpath);
			writer.open(outpath + filepath.filename().string(), BinaryIOMode::Write);

			writer.getWriter()->write(pDataBuf, inputsize);

			//if (file->posekeys.size())
			//{
			//	file->hdr.posekeyindex = writer.getPosition();
			//	for (int i = 0; i < file->posekeys.size(); i++)
			//		writer.write<float>(file->posekeys[i]);
			//}

			if (file->events.size())
			{
				writer.seek(file->hdr.eventindex, std::ios_base::_Seekbeg);
				file->hdr.eventindex = writer.getPosition();
				for (int i = 0; i < file->events.size(); i++)
					writer.write<mstudioeventv54_t>(file->events[i].event);

				//writer.seek(0, std::ios_base::_Seekend);
				//for (int i = 0; i < file->events.size(); i++)
				//	writer.writeString(file->events[i].string);
			}

			//if (file->activitymodifiers.size())
			//{
			//	file->hdr.activitymodifierindex = writer.getPosition();
			//	for (int i = 0; i < file->activitymodifiers.size(); i++)
			//		writer.write<mstudioactivitymodifierv53_t>(file->activitymodifiers[i].activitymodifier);
			//
			//	for (int i = 0; i < file->activitymodifiers.size(); i++)
			//		writer.writeString(file->activitymodifiers[i].string);
			//}

			//if (file->autolayers.size())
			//{
			//	file->hdr.autolayerindex = writer.getPosition();
			//	for (int i = 0; i < file->autolayers.size(); i++)
			//		writer.write<mstudioautolayerv54_t>(file->autolayers[i]);
			//}
			//
			//if (file->weightlist.size())
			//{
			//	file->hdr.weightlistindex = writer.getPosition();
			//	for (int i = 0; i < file->weightlist.size(); i++)
			//		writer.write<float>(file->weightlist[i]);
			//}
			//
			//file->hdr.animindexindex = writer.getPosition();
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
						reader.seek(offset + animdesc.desc.sectionindex, std::ios_base::_Seekbeg);
						//animdesc.desc.sectionframes = (animdesc.desc.numframes - 1);
						for (int i = 0; i < animdesc.sections.size(); i++)
							writer.write<mstudioanimsectionsv53_t>(animdesc.sections[i]);
					}

					writer.seek(offset, std::ios_base::_Seekbeg);
					writer.write<mstudioanimdescv54_t>(animdesc.desc);

					//for (int i = 0; i < animdesc.flagarray.size(); i++)
					//	writer.write<animflagarrayv54_t>(animdesc.flagarray[i]);

					// padding
					//uint8_t padding = 0;
					//writer.write<uint8_t>(padding);
					//
					//writer.write<char*>(animdesc.animationdata);
				}
			}
			//
			//if (file->blendgroups.size())
			//{
			//	file->hdr.animindexindex = writer.getPosition();
			//	for (int i = 0; i < file->blendgroups.size(); i++)
			//		writer.write<int>(file->blendgroups[i]);
			//}
			//
			//file->hdr.szlabelindex = writer.getPosition();
			//writer.writeString(file->szlabel);
			//
			//if (file->szactivity.length() > 0)
			//{
			//	file->hdr.szactivitynameindex = writer.getPosition();
			//	writer.writeString(file->szactivity);
			//}

			//if (file->SectionData.size())
			//{
			//	//file->hdr.animindexindex = writer.getPosition();
			//	for (int i = 0; i < file->SectionData.size(); i++)
			//		writer.write<char*>(file->SectionData[i]);
			//}
			//
			//// write new header data
			writer.seek(0, std::ios_base::_Seekbeg);
			writer.write<mstudioseqdesc_t>(file->hdr);

			writer.close();

		}
	}

	system("pause");
	return EXIT_SUCCESS;
}