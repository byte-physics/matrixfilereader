/*
	The file utils_bricklet.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/
#pragma once

/*
	Utility functions involving bricklets
*/

std::string viewTypeCodeToString(unsigned int idx);
std::string brickletTypeToString(unsigned int idx);

void setDataWaveNote(int brickletID, WaveClass &waveData);
void setOtherWaveNote(waveHndl waveHandle,int brickletID  = -1,int traceDir  = -1);
std::string getStandardWaveNote(int brickletID = -1, int traceDir = -1 );

bool isValidBrickletRange(double startID, double endID, int numberOfBricklets);
bool isValidBrickletID(int brickletID);
bool isValidTraceDir(int traceDir);

int createAndFillTextWave(const std::vector<std::string> &firstColumn, const std::vector<std::string> &secondColumn,
						  DataFolderHandle dataFolderHandle,const char *waveName, int brickletID, std::string &fullPathOfCreatedWaves);
