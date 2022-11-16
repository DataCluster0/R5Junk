#include <structs_VG.h>
#include "VGMagic.h"

using namespace std::filesystem;

int main(int argc, char* argv[]) {
	argc = 4;

	argv[1] = (char*)"H:\\GAMES\\R5pc_r5launch_N1094_CL456479_2019_10_30_05_20_PM\\exported_files+\\models\\error\\error_new.vg";

	if (argc < 2)
	{
		printf("\n==============================================\n");
		printf("$ Usage : R5VGOld.exe <source> <target> <options>\n");
		printf("==============================================\n\n");

		printf("$ Usage       : replace\n");
		printf("# Example     : R5Splinter.exe ./a.rmdl ./b.rmdl replace\n");
		printf("? Description : replaces targets bones with source bones\n\n");

		printf("==============================================\n\n");

		return 0;
	}

	if (argc < 3)
	{
		printf("Error : Target file not specified\n");
		return 0;
	}

	std::string SourceFile = std::string(argv[1]);
	//std::string TargetFile = std::string(argv[2]);

	auto sourcefile = std::filesystem::path(SourceFile);
	std::string SourceFileName = sourcefile.filename().string();

	//auto targetfile = std::filesystem::path(TargetFile);
	//std::string TargetFileName = targetfile.filename().string();

	if (!FILE_EXISTS(SourceFile))
	{
		printf("source file '%s' not found\n", SourceFileName.c_str());
		return 0;
	}

	//if (!FILE_EXISTS(TargetFile))
	//{
	//	printf("target file '%s' not found\n", TargetFileName.c_str());
	//	return 0;
	//}

	size_t SourceInputSize = Utils::GetFileSize(SourceFile);

	BinaryIO SourceReader;
	SourceReader.open(SourceFile, BinaryIOMode::Read);

	// read bone data
	//VGFileNew SourceData = ReadNewVG(SourceReader);

	SourceReader.close();


	//BinaryIO writer;
	//
	//std::string outpath = targetfile.parent_path().string() + "\\output\\";
	//
	//std::filesystem::create_directory(outpath);
	//writer.open(outpath + TargetFileName, BinaryIOMode::Write);
	//
	//writer.getWriter()->write(pDataBuf, TargetInputSize);

	// write new bone data
	
	//system("pause");
	return EXIT_SUCCESS;
}