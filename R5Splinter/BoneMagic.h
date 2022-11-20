#pragma once
#include <includes.h>
#include <structs_mdl.h>

FILE_Out STUDIOMDLReadBones(BinaryIO& Reader, std::string filename, int version);
void STUDIOMDLWriteBones(BinaryIO& writer, FILE_Out& Source, FILE_Out& Target, bool linear = true);