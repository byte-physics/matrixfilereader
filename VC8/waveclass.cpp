/*
	The file wavedata.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/

#include "header.h"

#include "waveclass.h"
#include "globaldata.h"

WaveClass::WaveClass(){

	m_traceDir = NO_TRACE;
	moreData=false;
	m_doublePtr=NULL;
	m_floatPtr=NULL;
	pixelSize=1;
	m_waveHandle=NULL;

	m_extrema = new ExtremaData();
}

WaveClass::~WaveClass(){

	delete m_extrema;
	m_extrema = NULL;
}

/*
	Set the extrema data, useful if WaveClass represents a complete bricklet
*/
void WaveClass::setExtrema(const ExtremaData& extremaData){

	m_extrema->setRawMin(extremaData.getRawMin());
	m_extrema->setRawMax(extremaData.getRawMax());

	m_extrema->setPhysValRawMax(extremaData.getPhysValRawMax());
	m_extrema->setPhysValRawMin(extremaData.getPhysValRawMin());
}


/*
	Make a connection between the waveHandle and this class
*/
void WaveClass::setWaveHandle(const waveHndl &waveHandle){

	if(WaveType(waveHandle) & NT_FP64){
		m_doublePtr = getWaveDataPtr<double>(waveHandle);
		m_floatPtr = NULL;

		if(m_doublePtr == NULL){
			outputToHistory("BUG: setWaveHandle(...) m_doublePtr is NULL, this should not happen...");
		}
		else{
			moreData = true;
			m_waveHandle = waveHandle;
		}
	}
	else if(WaveType(waveHandle) & NT_FP32){
		m_doublePtr = NULL;
		m_floatPtr = getWaveDataPtr<float>(waveHandle);

		if(m_floatPtr == NULL){
			outputToHistory("BUG: setWaveHandle(...) m_floatPtr is NULL, this should not happen...");
		}
		else{
			moreData = true;
			m_waveHandle = waveHandle;
		}
	}
	else if(WaveType(waveHandle) & NT_I32){
		m_doublePtr = NULL;
		m_floatPtr = NULL;

		m_waveHandle = waveHandle;
	}
	else{
		outputToHistory("wrong datatype in setWaveHandle");
	}
}

/*
	Set the name and trace direction of the wave, adds then the appropriate suffix to the wave name
*/
void WaveClass::setNameAndTraceDir(const std::string &basename, int traceDir){

	m_traceDir = traceDir;

	switch(m_traceDir){
		case NO_TRACE:
			m_wavename = basename;
			break;
		case TRACE_UP:
			m_wavename = basename + TRACE_UP_STRING;
			break;
		case RE_TRACE_UP:
			m_wavename = basename + RE_TRACE_UP_STRING;
			break;
		case TRACE_DOWN:
			m_wavename = basename + TRACE_DOWN_STRING;
			break;
		case RE_TRACE_DOWN:
			m_wavename = basename + RE_TRACE_DOWN_STRING;
			break;
		default:
			outputToHistory("tracedir is out of range");
			break;
	}
}

/*
	Output debug info
*/
void WaveClass::printDebugInfo(){
	sprintf(globDataPtr->outputBuffer,"%s: waveHandle=%p, float=%p, double=%p, moreData=%s",\
		m_wavename.empty() ? "empty" : m_wavename.c_str(), m_waveHandle, m_floatPtr, m_doublePtr, moreData ? "true" : "false");
	debugOutputToHistory(globDataPtr->outputBuffer);
}

/*
	Sets the complete wave to NaN
*/
void WaveClass::clearWave(){

	if(m_doublePtr){
		waveClearNaN64(m_doublePtr,WavePoints(m_waveHandle));
	}
	else if(m_floatPtr){
		waveClearNaN32(m_floatPtr,WavePoints(m_waveHandle));
	}
}

/*
	convenience wrapper
*/
void WaveClass::setWaveScaling(int dimension, const double* sfAPtr, const double* sfBPtr){
	ASSERT_RETURN_VOID(m_waveHandle);

	int ret = MDSetWaveScaling(m_waveHandle,dimension,sfAPtr,sfBPtr);
	if(ret != 0){
		outputToHistory(globDataPtr->outputBuffer);
		sprintf(globDataPtr->outputBuffer,"WaveClass::setWaveScaling returned error %d",ret);
	}
}

/*
	convenience wrapper
*/
void WaveClass::setWaveUnits(int dimension, const std::string& units){
	ASSERT_RETURN_VOID(m_waveHandle);

	int ret = MDSetWaveUnits(m_waveHandle,dimension,units.c_str());
	if(ret != 0){
		outputToHistory(globDataPtr->outputBuffer);
		sprintf(globDataPtr->outputBuffer,"WaveClass::setWaveUnits returned error %d",ret);
	}
}
