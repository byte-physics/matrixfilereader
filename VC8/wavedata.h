#pragma once

#include "header.h"
#include "extremadata.h"
#include <string>

class MyWave{

public:
	MyWave();
	~MyWave();

	void clearWave();
	void setWaveDataPtr(const waveHndl &waveHandle);
	void fillWave(const int &index, const int &rawValue, const double &scaledValue);
	void setNameAndTraceDir(const std::string &basename, const int &traceDir);
	void printDebugInfo();

	bool isEmpty(){ return m_wavename.empty(); }

	// getters
	const char* getWaveName(){ return m_wavename.c_str(); }
	waveHndl getWaveHandle(){ return m_waveHandle; }
	int getTraceDir(){ return m_traceDir; }


public:
	bool moreData;
	ExtremaData extrema;
	int pixelSize;

private:
	std::string m_wavename;
	int m_traceDir;
	waveHndl m_waveHandle;
	float* m_floatPtr;
	double* m_doublePtr;

};