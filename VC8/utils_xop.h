/*
	The file utils_xop.h is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/

/*
	Utility functions which are xop specific
*/

#pragma once

#include "header.h"

#include "waveclass.h"

#include <string>

const char outputFormat[]			= "%s\r";	//  outputToHistory
const char debugOutputFormat[]		= "DEBUG: %s\r"; // debugOutputToHistory

void debugOutputToHistory(const char *str, bool silent = false);
void outputToHistory(const char *str);

// be sure to check the return value for NULL
template <class T> T* getWaveDataPtr(waveHndl waveH){
	int accessMode = kMDWaveAccessMode0;
	int ret=-1;
	BCInt dataOffset;
	T *dataPtr;

	if(waveH == NULL){
		return NULL;
	}

	ret = MDAccessNumericWaveData(waveH, accessMode, &dataOffset);
	if(ret != 0 ){
		// throw here someting if you want to have it more C++-ish
		return NULL;
	}

	dataPtr = reinterpret_cast<T*>( reinterpret_cast<char*>(*waveH) + dataOffset );
	return dataPtr;
}

void setWaveNoteAsString(const std::string& waveNote, waveHndl waveHandle);

int stringVectorToTextWave(std::vector<std::string> &metaData,waveHndl &waveHandle);

void waveClearNaN64(double *wavePtr, const CountInt &length);
void waveClearNaN32(float  *wavePtr, const CountInt &length);

std::string getFullWavePath(const DataFolderHandle& df, const waveHndl& wv);
void appendToWaveList(const DataFolderHandle& df, const WaveClass& wave, std::string& waveList);
void appendToWaveList(const DataFolderHandle& df, const waveHndl& wv, std::string& waveList);

void convertHandleToString(Handle strHandle,std::string &str);

