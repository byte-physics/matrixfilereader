/*
	The file utils_bricklet.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/

#pragma once

#include <limits>
#include "waveclass.h"

std::vector<std::wstring> getAllAxesNames(void *pBricklet);
std::string viewTypeCodeToString(int idx);

void setDataWaveNote(int brickletID, WaveClass &waveData);
void setOtherWaveNote(waveHndl waveHandle,int brickletID  = -1,int traceDir  = -1);
std::string getStandardWaveNote(int brickletID = -1, int traceDir = -1 );

bool isValidBrickletRange(double startID, double endID, int numberOfBricklets);


