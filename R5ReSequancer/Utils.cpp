#include "includes.h"
#include "Utils.h"

// purpose: gets size of the specified file
// returns: file size
uintmax_t Utils::GetFileSize(std::string filename)
{
	try {
		return std::filesystem::file_size(filename);
	}
	catch (std::filesystem::filesystem_error& e) {
		std::cout << e.what() << '\n';
		exit(0);
	}
}

// purpose: pad buffer to the specified alignment
// returns: new buffer size
size_t Utils::PadBuffer(char** buf, size_t size, size_t alignment)
{
	size_t extra = alignment - (size % alignment);
	size_t newSize = size + extra;

	char* newbuf = new char[newSize] {};
	memcpy_s(newbuf, size, *buf, size);

	delete[] * buf;

	*buf = newbuf;
	return newSize;
}

// purpose: get current system time as FILETIME
FILETIME Utils::GetFileTimeBySystem()
{
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	return ft;
}

// purpose: add backslash to the end of the string if not already present
void Utils::AppendSlash(std::string& in)
{
	char lchar = in[in.size() - 1];
	if (lchar != '\\' && lchar != '/')
		in.append("\\");
}

std::string Utils::ChangeExtension(const std::string& in, const std::string& ext)
{
	return std::filesystem::path(in).replace_extension(ext).u8string();
}