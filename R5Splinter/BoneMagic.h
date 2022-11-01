#pragma once
#include <includes.h>
#include "structs.h"

FILE_Out STUDIOMDLReadBones(BinaryIO& Reader, std::string filename);
void STUDIOMDLWriteBones(BinaryIO& writer, FILE_Out& Source, FILE_Out& Target, bool linear = true);