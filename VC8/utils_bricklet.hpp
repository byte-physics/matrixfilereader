/*
  The file utils_bricklet.hpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#pragma once

/*
  Utility functions involving bricklets
*/
std::string viewTypeCodeToString(unsigned int idx);
std::string brickletTypeToString(unsigned int idx);

void setDataWaveNote(int brickletID, const WaveClass& waveData);
void setOtherWaveNote(waveHndl waveHandle, int brickletID  = -1, int traceDir  = -1);
std::string getStandardWaveNote(int brickletID = -1, int traceDir = -1);

bool isValidBrickletRange(double startID, double endID, int numberOfBricklets);
bool isValidBrickletID(int brickletID);
bool isValidTraceDir(int traceDir);

int createAndFillTextWave(DataFolderHandle baseFolderHandle, const std::vector<std::pair<std::string,std::string> >& data,
                          DataFolderHandle dataFolderHandle, const char* waveName, int brickletID, std::string& waveNameList);

class Vernissage::Session;
Vernissage::Session* getVernissageSession();
