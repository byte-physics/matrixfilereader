/*
	The file brickletconverter.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt	in the source folder for details.
*/

#pragma once

#include "header.h"

int createWaves(DataFolderHandle dataFolderHandle, const char *baseName, int brickletID, bool resampleData, int pixelSize, std::string &fullPathOfCreatedWaves);

