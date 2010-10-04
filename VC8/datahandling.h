#pragma once

#include "header.h"

struct waveDataPtr{
	float* flt;
	double* dbl;
	bool moreData;
};

struct extremaData{
	int		rawMin;
	int		rawMax;
	double	physValRawMin;
	double	physValRawMax;
};

void clearWave(waveHndl waveHandle,long waveSize);
void setWaveDataPtr(waveDataPtr &waveData,const waveHndl &waveHandle);

void __forceinline fillWave(const waveDataPtr &waveData,const int index, const double value,const bool isDoubleWaveType);
void __forceinline setExtremaValue(extremaData *extrema,const int rawValue,const double scaledValue);

int createAndFillDataWave(DataFolderHandle dataFolderHandle, const char *waveName, int brickletID);