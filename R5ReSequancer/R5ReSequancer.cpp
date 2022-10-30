#include <includes.h>
#include <Utils.h>
#include "structs.h"

using namespace std::filesystem;

int main(int argc, char* argv[]) {
	argv[0] = (char*)"H:\\GAMES\\R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM\\R_Repositories\\R5Junk\\x64\\Debug\\ads_in.rseq";
	argc = 1;

	for (int i = 0; i < argc; i++)
	{
		auto tmpfilepath = std::string((const char*)argv[i]);
		if (tmpfilepath.find(".rseq") == -1)
			continue;
		ASEQ_Out* file = new ASEQ_Out();

		auto filepath = std::filesystem::path(tmpfilepath);

		printf("========================\n");
		printf("%s\n", filepath.filename().string().c_str());

		char* pExtDataBuf = nullptr;
		std::string extdatapath = Utils::ChangeExtension(tmpfilepath, "rseqdata");
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
				for (int i = 0; i < posekeys; i++)
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

					uint64_t nameoffset = (seqdesc.numevents * sizeof(mstudioeventv54_t)) + str_offset;
					v7.szeventindex = nameoffset - (i * sizeof(mstudioeventv54_t));

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

			reader.seek(seqdesc.szlabelindex, std::ios_base::_Seekbeg);
			file->szlabel = reader.readString();

			reader.seek(seqdesc.szactivitynameindex, std::ios_base::_Seekbeg);
			file->szactivity = reader.readString();

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

				reader.seek(animdesc_offset + animdesc.sznameindex, std::ios_base::_Seekbeg);
				animdata.name = reader.readString();

				animdesc.sznameindex = +(sizeof(mstudioanimdescv54_t_v121) - sizeof(mstudioanimdescv54_t));

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

							ASEQ_Out_section sectiondata;
							sectiondata.data = section;

							animdata.sections.push_back(sectiondata);
						}

						for (auto& section : animdata.sections)
						{
							if (section.data.isExternal)
							{
								extreader.seek(section.data.animindex);

								uint64_t boneflagpos = extreader.tell();
								for (int i = 0; i < numbones; i++)
								{
									animflagarrayv54_t boneflag = extreader.read<animflagarrayv54_t>();
									section.flagarray.push_back(boneflag);
								}

								reader.seek(boneflagpos + (numbones / 2));
								for (int j = 0; j < numbones; j++)
								{
									animflagarrayv54_t boneflag = section.flagarray[j];

									if (boneflag.FlagCount() > 0)
										section.animdata.push_back(Read_mstudio_rle_anim_t(extreader, boneflag));
								}
							}
							else
							{
								reader.seek(animdesc_offset + section.data.animindex, std::ios_base::_Seekbeg);

								uint64_t boneflagpos = reader.tell();
								for (int i = 0; i < numbones; i++)
								{
									animflagarrayv54_t boneflag = reader.read<animflagarrayv54_t>();
									section.flagarray.push_back(boneflag);
								}

								reader.seek(boneflagpos + (numbones / 2));
								for (int j = 0; j < numbones; j++)
								{
									animflagarrayv54_t boneflag = section.flagarray[j];

									if (boneflag.FlagCount() > 0)
										section.animdata.push_back(Read_mstudio_rle_anim_t(reader, boneflag));
								}
							}
						}
					}
					else
					{
						reader.seek(animdesc_offset + animdesc.animindex, std::ios_base::_Seekbeg);

						uint64_t boneflagpos = reader.tell();
						for (int i = 0; i < numbones; i++)
						{
							animflagarrayv54_t boneflag = reader.read<animflagarrayv54_t>();
							animdata.flagarray.push_back(boneflag);
						}

						reader.seek(boneflagpos + (numbones / 2));
						for (int j = 0; j < numbones; j++)
						{
							animflagarrayv54_t boneflag = animdata.flagarray[j];

							if (boneflag.FlagCount() > 0)
								animdata.animdata.push_back(Read_mstudio_rle_anim_t(reader, boneflag));
						}
					}
				}

				file->animdescs.push_back(animdata);
			}

			reader.close();

			if (IsExternal)
				printf("%s -> external data detected...\n", filepath.filename().string().c_str());

			if (IsSegmented)
				printf("%s -> segmented data detected...\n", filepath.filename().string().c_str());

			BinaryIO writer;
			std::string outpath = filepath.parent_path().string() + "\\downgraded\\";

			std::filesystem::create_directory(outpath);
			writer.open(outpath + filepath.filename().string(), BinaryIOMode::Write);

			//writer.getWriter()->write(pDataBuf, inputsize);
			//writer.seek(inputsize, std::ios_base::_Seekbeg);
			//writer.getWriter()->write(pExtDataBuf, extinputsize);
			writer.write<mstudioseqdesc_t>(file->hdr);

			if (file->posekeys.size())
			{
				file->hdr.posekeyindex = writer.tell();
				for (auto& posekey : file->posekeys)
					writer.write<float>(posekey);
			}

			if (file->events.size())
			{
				file->hdr.eventindex = writer.tell();
				for (auto& event : file->events)
					writer.write<mstudioeventv54_t>(event.event);
			}

			if (file->weightlist.size())
			{
				file->hdr.weightlistindex = writer.tell();
				for (auto& weight : file->weightlist)
					writer.write<float>(weight);
			}

			file->hdr.activitymodifierindex = writer.tell();
			if (file->activitymodifiers.size())
			{
				for (auto& actmod : file->activitymodifiers)
					writer.write<mstudioactivitymodifierv53_t>(actmod.activitymodifier);

				for (auto& actmod : file->activitymodifiers)
					writer.writeString(actmod.string);
			}

			if (file->autolayers.size())
			{
				file->hdr.autolayerindex = writer.tell();
				for (auto& autolayer : file->autolayers)
					writer.write<mstudioautolayerv54_t>(autolayer);
			}

			// write blendgroup offsets first
			if (file->blendgroups.size())
			{
				file->hdr.animindexindex = writer.tell();
				int temp = 0;
				for (auto& blend : file->blendgroups)
					writer.write<int>(temp);
			}

			std::vector<int> newbodygroups;
			if (file->animdescs.size())
			{
				uint64_t animdatasize = 0;

				for (auto& animdesc : file->animdescs)
				{
					// set blendgroup data
					uint64_t animdescpos = writer.tell();
					newbodygroups.push_back(animdescpos);

					// prewrite header
					writer.write<mstudioanimdescv54_t>(animdesc.desc);

					animdesc.desc.ikruleindex = writer.tell() - animdescpos; // ik rule offset
					for (auto& ikrule : animdesc.ikrules)
						writer.write<mstudioikrulev54_t>(ikrule);

					uint64_t senctionpos = writer.tell();

					if (animdesc.sections.size())
					{
						// keep old pos for later overwrite
						uint64_t sectionpos = writer.tell();
						animdesc.desc.sectionindex = sectionpos - animdescpos;
						for (auto& section : animdesc.sections)
						{
							int temp = 0;
							writer.write<int>(temp);
						}

						for (auto& section : animdesc.sections)
						{
							uint64_t boneflagoffset = writer.tell();
							
							// write bone flag array
							for (auto& boneflag : section.flagarray)
								writer.write<animflagarrayv54_t>(boneflag);
							
							writer.seek(boneflagoffset + animdesc.flagarray.size());
							int64_t animdataoffset = writer.tell();
							
							for (auto& data : section.animdata)
							{
								auto headerpos = writer.tell();
								writer.write<mstudio_rle_animv54_t>(data.hdr);
							
								if (data.hdr.STUDIO_ANIM_ANIMPOS_54)
									writer.write<float>(data.posscale);

								uint64_t posoffset = 0;
								if (data.boneflags.STUDIO_ANIM_ANIMPOS_54)
								{
									if (data.hdr.STUDIO_ANIM_ANIMPOS_54)
									{
										posoffset = writer.tell();
										writer.write<mstudioanim_valueptrv54_t>(data.animpos);
									}
									else writer.write<uint32_t>(data.rawpos);
								}

								uint64_t rotoffset = 0;
								if (data.boneflags.STUDIO_ANIM_ANIMROT_54)
								{
									if (data.hdr.STUDIO_ANIM_ANIMROT_54)
									{
										rotoffset = writer.tell();
										writer.write<mstudioanim_valueptrv54_t>(data.animrot);
									}
									else writer.write<uint64_t>(data.rawrot);
								}

								uint64_t angleoffset = 0;
								if (data.boneflags.STUDIO_ANIM_ANIMSCALE_54)
								{
									if (data.hdr.STUDIO_ANIM_ANIMSCALE_54)
									{
										angleoffset = writer.tell();
										writer.write<mstudioanim_valueptrv54_t>(data.animscale);
									}
									else writer.write<uint32_t>(data.rawscale);
								}

								if (data.hdr.STUDIO_ANIM_ANIMPOS_54)
								{
									posoffset = writer.tell();
									writer.write<mstudioanim_valueptrv54_t>(data.animpos);
								}

								if (data.hdr.STUDIO_ANIM_ANIMROT_54)
								{
									if (data.animrot.fields.offset)
									{
										data.animrot.fields.offset = (int32_t)(writer.tell() - rotoffset);
										writer.write<mstudioanimvalue_t>(data.animvaluerot[0]);
									}

									if (data.animrot.fields.offset1)
									{
										data.animrot.fields.offset1 = (int32_t)(writer.tell() - rotoffset);
										writer.write<mstudioanimvalue_t>(data.animvaluerot[1]);
									}

									if (data.animrot.fields.offset2)
									{
										data.animrot.fields.offset2 = (int32_t)(writer.tell() - rotoffset);
										writer.write<mstudioanimvalue_t>(data.animvaluerot[2]);
									}

									uint64_t oldoffset = writer.tell();

									writer.seek(rotoffset);
									writer.write<mstudioanim_valueptrv54_t>(data.animrot);
									writer.seek(oldoffset);
								}
							
								uint64_t oldpos = writer.tell();
							
								writer.seek(headerpos);
							
								data.hdr.size = (uint16_t)(oldpos - headerpos);
								writer.write<mstudio_rle_animv54_t>(data.hdr);
							
								writer.seek(oldpos);
							}
							
							animdesc.desc.animindex = boneflagoffset - animdescpos;
						}

						auto oldpos = writer.tell();
						writer.seek(sectionpos);
						for (auto& section : animdesc.sections)
							writer.write<int>(section.data.animindex);

						writer.seek(oldpos);
					}
					else if (animdesc.animdata.size())
					{
						uint64_t boneflagoffset = writer.tell();

						// write bone flag array
						for (auto& boneflag : animdesc.flagarray)
							writer.write<animflagarrayv54_t>(boneflag);

						writer.seek(boneflagoffset + (animdesc.flagarray.size() / 2));
						int64_t animdataoffset = writer.tell();

						for (auto& data : animdesc.animdata)
						{
							auto headerpos = writer.tell();
							writer.write<mstudio_rle_animv54_t>(data.hdr);

							if (data.hdr.STUDIO_ANIM_ANIMPOS_54)
								writer.write<float>(data.posscale);

							uint64_t posoffset = 0;
							if (data.boneflags.STUDIO_ANIM_ANIMPOS_54)
							{
								if (data.hdr.STUDIO_ANIM_ANIMPOS_54)
								{
									posoffset = writer.tell();
									writer.write<mstudioanim_valueptrv54_t>(data.animpos);
								}
								else writer.write<uint32_t>(data.rawpos);
							}

							uint64_t rotoffset = 0;
							if (data.boneflags.STUDIO_ANIM_ANIMROT_54)
							{
								if (data.hdr.STUDIO_ANIM_ANIMROT_54)
								{
									rotoffset = writer.tell();
									writer.write<mstudioanim_valueptrv54_t>(data.animrot);
								}
								else writer.write<uint64_t>(data.rawrot);
							}

							uint64_t angleoffset = 0;
							if (data.boneflags.STUDIO_ANIM_ANIMSCALE_54)
							{
								if (data.hdr.STUDIO_ANIM_ANIMSCALE_54)
								{
									angleoffset = writer.tell();
									writer.write<mstudioanim_valueptrv54_t>(data.animscale);
								}
								else writer.write<uint32_t>(data.rawscale);
							}

							if (data.hdr.STUDIO_ANIM_ANIMPOS_54)
							{
								posoffset = writer.tell();
								writer.write<mstudioanim_valueptrv54_t>(data.animpos);
							}

							if (data.hdr.STUDIO_ANIM_ANIMROT_54)
							{
								if (data.animrot.fields.offset)
								{
									data.animrot.fields.offset = (int32_t)(writer.tell() - rotoffset);
									writer.write<mstudioanimvalue_t>(data.animvaluerot[0]);
								}

								if (data.animrot.fields.offset1)
								{
									data.animrot.fields.offset1 = (int32_t)(writer.tell() - rotoffset);
									writer.write<mstudioanimvalue_t>(data.animvaluerot[1]);
								}

								if (data.animrot.fields.offset2)
								{
									data.animrot.fields.offset2 = (int32_t)(writer.tell() - rotoffset);
									writer.write<mstudioanimvalue_t>(data.animvaluerot[2]);
								}

								uint64_t oldoffset = writer.tell();

								writer.seek(rotoffset);
								writer.write<mstudioanim_valueptrv54_t>(data.animrot);
								writer.seek(oldoffset);
							}

							animdesc.desc.animindex = boneflagoffset - animdescpos;

							uint64_t oldpos = writer.tell();
							data.hdr.size = (uint16_t)(oldpos - headerpos);
							//
							writer.seek(headerpos);
							writer.write<mstudio_rle_animv54_t>(data.hdr);
							writer.seek(oldpos);
						}
					}

					auto oldpos = writer.tell();

					// write new animdesc header
					writer.seek(animdescpos);
					writer.write<mstudioanimdescv54_t>(animdesc.desc);
					writer.seek(oldpos);
				}
			}

			uint64_t oldpos = writer.tell();

			// write new blendgroup data
			if (newbodygroups.size())
			{
				writer.seek(file->hdr.animindexindex);
				for (auto& blend : newbodygroups)
					writer.write<int>(blend);
			}

			// return to old pos
			writer.seek(oldpos);

			file->hdr.szlabelindex = writer.tell();
			writer.writeString(file->szlabel);

			if (file->szactivity.length() > 0)
			{
				file->hdr.szactivitynameindex = writer.tell();
				writer.writeString(file->szactivity);
			}

			uint64_t nameoffset = writer.tell();
			if (file->events.size())
			{
				uint64_t oldpos = reader.tell();
				for (int i = 0; i < file->events.size(); i++)
				{
					uint64_t animdesc_offset = file->hdr.eventindex + (i * sizeof(mstudioeventv54_t));
					auto eventdesc = file->events[i];

					eventdesc.event.szeventindex = writer.tell() - animdesc_offset;
					writer.writeString(eventdesc.string);
					nameoffset += eventdesc.string.length() + 1;

					writer.seek(animdesc_offset);
					writer.write<mstudioeventv54_t>(eventdesc.event);
					writer.seek(nameoffset);
				}
			}

			nameoffset = writer.tell();
			for (int i = 0; i < newbodygroups.size(); i++)
			{
				uint64_t animdesc_offset = newbodygroups[i];
				auto animdesc = file->animdescs[i];

				animdesc.desc.sznameindex = writer.tell() - animdesc_offset;
				writer.writeString(animdesc.name);
				nameoffset += animdesc.name.length() + 1;

				writer.seek(animdesc_offset);
				writer.write<mstudioanimdescv54_t>(animdesc.desc);
				writer.seek(nameoffset);
			}

			file->hdr.unkindex = file->hdr.szlabelindex;

			//// write new header data
			writer.seek(0);
			writer.write<mstudioseqdesc_t>(file->hdr);

			writer.close();
		}
	}

	//system("pause");
	return EXIT_SUCCESS;
}