#include "BoneMagic.h"

using namespace std::filesystem;

int main(int argc, char* argv[]) {
	if (argc < 2)
	{
		printf("\n==============================================\n");
		printf("$ Usage : R5Splinter.exe <source> <target> <source version> <options>\n");
		printf("==============================================\n\n");

		printf("$ Usage       : replace\n");
		printf("# Example     : R5Splinter.exe ./a.rmdl ./b.rmdl 10  replace\n");
		printf("? Description : replaces targets bones with source bones\n\n");

		printf("==============================================\n\n");

		printf("$ Usage       : move -> <source bone name> <pos vector>\n");
		printf("# Example     : Example : R5Splinter.exe ./a.rmdl ./b.rmdl 10 move ja_c_propGun 0 0 20\n");
		printf("? Description : moves the specified bone along with it's children\n\n");

		printf("==============================================\n\n");

		printf("$ Usage       : movetobone -> <source bone name> <target bone name>\n");
		printf("# Example     : Example : R5Splinter.exe ./a.rmdl ./b.rmdl 10 movetobone ja_c_propGun jx_c_origin\n");
		printf("? Description : moves the source bone along with it's children to target bones location\n\n");

		printf("==============================================\n\n");

		return 0;
	}

	if (argc < 3)
	{
		printf("Error : Target file not specified\n");
		return 0;
	}

	if (argc < 4)
	{
		printf("Error : version not specified\n");
		return 0;
	}

	if (argc < 5)
	{
		printf("Error : options not specified\n");
		return 0;
	}

	//while (!IsDebuggerPresent())
	//	::Sleep(100);

	std::string SourceFile = std::string(argv[1]);
	std::string TargetFile = std::string(argv[2]);
	int version = stoi(std::string(argv[3]));

	auto sourcefile = std::filesystem::path(SourceFile);
	std::string SourceFileName = sourcefile.filename().string();

	auto targetfile = std::filesystem::path(TargetFile);
	std::string TargetFileName = targetfile.filename().string();

	if (!FILE_EXISTS(SourceFile))
	{
		printf("source file '%s' not found\n", SourceFileName.c_str());
		return 0;
	}

	if (!FILE_EXISTS(TargetFile))
	{
		printf("target file '%s' not found\n", TargetFileName.c_str());
		return 0;
	}

	std::string option = std::string(argv[4]);

	size_t SourceInputSize = Utils::GetFileSize(SourceFile);

	BinaryIO SourceReader;
	SourceReader.open(SourceFile, BinaryIOMode::Read);
	// read bone data
	FILE_Out SourceBoneData = STUDIOMDLReadBones(SourceReader, SourceFileName, version);

	size_t TargetInputSize = Utils::GetFileSize(TargetFile);
	char* pDataBuf = new char[TargetInputSize];

	BinaryIO TargetReader;
	TargetReader.open(TargetFile, BinaryIOMode::Read);
	TargetReader.getReader()->read(pDataBuf, TargetInputSize);
	TargetReader.seek(0);
	FILE_Out TargetBoneData = STUDIOMDLReadBones(TargetReader, TargetFileName, 10);

	bool linear = true;

	// check if the target is a rig
	if (TargetFile.find(".rrig") != -1)
		linear = false;

	if (option == "move")
	{
		std::string bonename = std::string(argv[5]);
		int bone = SourceBoneData.FindBoneByName(bonename);

		if (bone != -1)
		{
			if (argc >= 9)
			{
				float x = ::atof(std::string(argv[6]).c_str());
				float y = ::atof(std::string(argv[7]).c_str());
				float z = ::atof(std::string(argv[8]).c_str());

				printf("Moving %s : %6.4lf, %6.4lf, %6.4lf\n", bonename.c_str(), x, y, z);

				SourceBoneData.MoveChildBones(bone, Vector3(x, y, z));
			}
			else
			{
				printf("move xyz not found\n");
				return 0;
			}
		}
		else return 0;
	}
	else if (option == "movetobone")
	{
		std::string sourcename = std::string(argv[4]);
		int sourcebone = SourceBoneData.FindBoneByName(sourcename);

		if (argc < 6)
		{
			printf("Target bone not provided\n");
			return 0;
		}

		std::string targetname = std::string(argv[5]);
		int targetbone = TargetBoneData.FindBoneByName(targetname);

		if (sourcebone != -1 && targetbone != -1)
		{
			if (argc >= 6)
			{
				Vector3 Source = SourceBoneData.GetPosById(sourcebone);
				Vector3 Target = TargetBoneData.GetPosById(targetbone);

				Vector3 Result = -Source + -Target;

				printf("Source : %6.4lf, %6.4lf, %6.4lf\nTarget : %6.4lf, %6.4lf, %6.4lf\nResult : %6.4lf, %6.4lf, %6.4lf\n", Source.X, Source.Y, Source.Z, Target.X, Target.Y, Target.Z, Result.X, Result.Y, Result.Z);

				SourceBoneData.MoveChildBones(sourcebone, Result);
			}
		}
		else return 0;
	}
	else if (option == "replace")
	{
		printf("'%s' replaced with '%s'\n",TargetFileName.c_str(), SourceFileName.c_str());
	}
	else {
		printf("unknown option '%s' found\n", option.c_str());
		return 0;
	}

	BinaryIO writer;

	std::string outpath = targetfile.parent_path().string() + "\\output\\";

	std::filesystem::create_directory(outpath);
	writer.open(outpath + TargetFileName, BinaryIOMode::Write);

	writer.getWriter()->write(pDataBuf, TargetInputSize);

	// write new bone data
	STUDIOMDLWriteBones(writer, SourceBoneData, TargetBoneData, linear);

	//system("pause");
	return EXIT_SUCCESS;
}