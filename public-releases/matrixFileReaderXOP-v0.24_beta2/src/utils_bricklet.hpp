/*
  The file utils_bricklet.hpp is part of the "MatrixFileReader XOP".
  It is licensed under the LGPLv3 with additional permissions,
  see License.txt in the source folder for details.
*/
#pragma once
#include "ForwardDecl.hpp"

/*
  Utility functions involving bricklets
*/
std::string viewTypeCodeToString(unsigned int idx);
std::string brickletTypeToString(unsigned int idx);

void setDataWaveNote(int brickletID, const Wave& waveData);
void setOtherWaveNote(waveHndl waveHandle, int brickletID  = -1, int traceDir  = -1, std::string suffix = std::string());

bool isValidBrickletRange(double startID, double endID, int numberOfBricklets);
bool isValidBrickletID(int brickletID);
bool isValidTraceDir(int traceDir);

int isOverwriteEnabled();
int getIgorWaveType();

int createAndFillTextWave(DataFolderHandle baseFolderHandle, const StringPairVector& data,
                          DataFolderHandle dataFolderHandle, const char* waveName, int brickletID, std::string& waveNameList);

Vernissage::Session* getVernissageSession();
std::vector<void*> getBrickletSeries(void* rawBrickletPtr);

void loadAllBrickletMetaData();
void loadAllBrickletData();
void loadAllBrickletDataAndMetaData();
