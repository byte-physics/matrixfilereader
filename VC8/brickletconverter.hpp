/*
  The file brickletconverter.h is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/

/*
  The heart of the XOP. Here the 1D arrays are converted to (multiple) Igor waves having the data in the correct form.
  Holds the implementations for 1D, 2D and 3D.
*/
#pragma once

#include <string>

int createRawDataWave(DataFolderHandle baseFolderHandle, DataFolderHandle dataFolderHandle, const char *waveName,
                      int brickletID, std::string &waveNameList);
int createWaves(DataFolderHandle baseFolderHandle, DataFolderHandle dataFolderHandle, const char *baseName,
                int brickletID, bool resampleData, int pixelSize, std::string &waveNameList);
