#pragma once

namespace Utils
{
	uintmax_t GetFileSize(std::string filename);

	size_t PadBuffer(char** buf, size_t size, size_t alignment);

	FILETIME GetFileTimeBySystem();

	void AppendSlash(std::string& in);

	std::string ChangeExtension(const std::string& in, const std::string& ext);
};

#define FILE_EXISTS(path) std::filesystem::exists(path)

#define REQUIRE_FILE(path) \
	if(!FILE_EXISTS(path)) \
		Error("Unable to find required file '%s'\n", std::string(path).c_str())