/*
	The file utils_bricklet.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/

#include "stdafx.h"

#include "utils_bricklet.hpp"
#include "waveclass.hpp"
#include "globaldata.hpp"
#include "utils_generic.hpp"

/*
	Create a two column text wave from two string vectors
*/
int createAndFillTextWave(const std::vector<std::string>& firstColumn, const std::vector<std::string>&secondColumn,
						  DataFolderHandle dataFolderHandle,const char *waveName, int brickletID, std::string &fullPathOfCreatedWaves){

	std::vector<std::string> allColumns;
	waveHndl waveHandle;
	int ret=-1;
	CountInt dimensionSizes[MAX_DIMENSIONS+1];
	MemClear(dimensionSizes, sizeof(dimensionSizes));

	// create 2D textwave with firstColumn.size() rows and 2 columns
	if(firstColumn.empty() || secondColumn.empty() ){
		HISTPRINT("BUG: list sizes may not be zero: first column size %d, second column size %d",firstColumn.size(), secondColumn.size());
		return UNKNOWN_ERROR;
	}
	else if(firstColumn.size() == secondColumn.size() ){
		dimensionSizes[ROWS] = firstColumn.size();
	}
	else{
		HISTPRINT("BUG: size mismatch of first column %d and second column %d",firstColumn.size(), secondColumn.size());
		return UNKNOWN_ERROR;
	}

	dimensionSizes[COLUMNS]=2;
	
	ret = MDMakeWave(&waveHandle,waveName,dataFolderHandle,dimensionSizes,TEXT_WAVE_TYPE,GlobalData::Instance().overwriteEnabledAsInt());

	if(ret == NAME_WAV_CONFLICT){
		DEBUGPRINT("Wave %s already exists.",waveName);
				return WAVE_EXIST;
	}
	else if(ret != 0 ){
		HISTPRINT("Error %d in creating wave %s.",ret, waveName);
		return UNKNOWN_ERROR;
	}
	ASSERT_RETURN_ONE(waveHandle);

	// copy the strings of both columns into a new vector
	// so that they are then 1D
	// first firstColumn, then secondColumn
	try{
		allColumns.reserve(firstColumn.size() + secondColumn.size());
	}
	catch(CMemoryException* e){
		e->Delete();
		HISTPRINT("Out of memory in createAndFillTextWave()");
		return UNKNOWN_ERROR;
	}
	allColumns.insert(allColumns.begin(),firstColumn.begin(),firstColumn.end());
	allColumns.insert(allColumns.end(),secondColumn.begin(),secondColumn.end());

	ret = stringVectorToTextWave(allColumns,waveHandle);

	if(ret != 0){
		HISTPRINT("stringVectorToTextWave returned %d",ret);
		return ret;
	}
	setOtherWaveNote(waveHandle,brickletID);
	appendToWaveList(dataFolderHandle,waveHandle,fullPathOfCreatedWaves);

	return 0;
}

/*
	Convert a vernissage viewtype code to a string
*/
std::string viewTypeCodeToString(unsigned int idx){
	
	std::vector<std::string> names;

	names.push_back(VTC_OTHER_STRING);
	names.push_back(VTC_SIMPLE2D_STRING);
	names.push_back(VTC_SIMPLE1D_STRING);
	names.push_back(VTC_FWDBWD2D_STRING);
	names.push_back(VTC_2DOF3D_STRING);
	names.push_back(VTC_SPECTROSCOPY_STRING);
	names.push_back(VTC_FORCECURVE_STRING);
	names.push_back(VTC_1DPROFILE_STRING);
	names.push_back(VTC_INTERFEROMETER);
	names.push_back(VTC_CONTINUOUSCURVE);
	names.push_back(VTC_PHASEAMPLITUDECURVE);
	names.push_back(VTC_CURVESET);
	names.push_back(VTC_PARAMETERISEDCURVESET);
	names.push_back(VTC_DISCRETEENERGYMAP);
	names.push_back(VTC_ESPIMAGEMAP);

	if(idx < 0 || idx >= names.size()){
		DEBUGPRINT("BUG: viewTypeCodeToString got %d as parameter, but it should be between 0 and %d",idx,names.size()-1);
				return std::string();
	}
	else{
		return names.at(idx);
	}
}

/*
	Convert the bricklet type enumeration value into a human readable string
*/
std::string brickletTypeToString(unsigned int idx){

	std::vector<std::string> names;

	names.push_back(BTC_UNKNOWN);
	names.push_back(BTC_SPMSPECTROSCOPY);
	names.push_back(BTC_ATOMMANIPULATION);
	names.push_back(BTC_1DCURVE);
	names.push_back(BTC_SPMIMAGE);
	names.push_back(BTC_PATHSPECTROSCOPY);
	names.push_back(BTC_ESPREGION);
	names.push_back(BTC_VOLUMECITS);
	names.push_back(BTC_DISCRETEENERGYMAP);
	names.push_back(BTC_FORCECURVE);
	names.push_back(BTC_PHASEAMPLITUDECURVE);
	names.push_back(BTC_SIGNALOVERTIME);
	names.push_back(BTC_RAWPATHSPECTROSCOPY);
	names.push_back(BTC_ESPSNAPSHOTSEQUENCE);
	names.push_back(BTC_ESPIMAGEMAP);

	if(idx < 0 || idx >= names.size()){
		DEBUGPRINT("BUG: brickletTypeToString got %d as parameter, but it should be between 0 and %d",idx,names.size()-1);
				return std::string();
	}
	else{
		return names.at(idx);
	}
}

/*
	Set the appropriate wave note for data waves
*/
void setDataWaveNote(int brickletID, WaveClass &waveData){

	std::string	waveNote = getStandardWaveNote(brickletID,waveData.getTraceDir());

	waveNote.append("rawMin="    + anyTypeToString<int>(waveData.getExtrema().getRawMin())    + "\r");
	waveNote.append("rawMax="	 + anyTypeToString<int>(waveData.getExtrema().getRawMax())	 + "\r");
	
	waveNote.append("physicalValueOfRawMin=" + anyTypeToString<double>(waveData.getExtrema().getPhysValRawMin()) + "\r");
	waveNote.append("physicalValueOfRawMax=" + anyTypeToString<double>(waveData.getExtrema().getPhysValRawMax()) + "\r");
	waveNote.append("pixelSize=" + anyTypeToString<int>(waveData.pixelSize) + "\r");

	setWaveNoteAsString(waveNote, waveData.getWaveHandle());
}

/*
	Set the appropriate wave note for the other waves (bricklet metadata, resultfile meta data, overviewtable)
*/

void setOtherWaveNote(waveHndl waveHandle,int brickletID /*= -1*/,int traceDir  /*= -1*/){

	setWaveNoteAsString(getStandardWaveNote(brickletID, traceDir),waveHandle);
}

/*
	Return a string containing the standard wave note part
*/
std::string getStandardWaveNote(int brickletID /* = -1 */, int traceDir /* = -1 */){

	std::string waveNote;

	waveNote.append(RESULT_FILE_NAME_KEY + "=" + GlobalData::Instance().getFileName() + "\r");
	waveNote.append(RESULT_DIR_PATH_KEY +"=" + GlobalData::Instance().getDirPath() + "\r");

	if(isValidBrickletID(brickletID)){
		waveNote.append(BRICKLET_ID_KEY + "=" + anyTypeToString<int>(brickletID) + "\r");
	}
	else{
		waveNote.append(BRICKLET_ID_KEY + "=\r");
	}

	if(isValidTraceDir(traceDir)){
		waveNote.append(TRACEDIR_ID_KEY + "=" + anyTypeToString<int>(traceDir) + "\r");
	}
	else{
		waveNote.append(TRACEDIR_ID_KEY + "=\r");
	}

	waveNote.append("xopVersion=" + std::string(MatrixFileReader_XOP_VERSION_STR) + "\r");
	waveNote.append("vernissageVersion=" + GlobalData::Instance().getVernissageVersion() + "\r");

	return waveNote;
}

/*
	Check if the bricklet range startID-endID is valid
*/
bool isValidBrickletRange(double startID, double endID, int numberOfBricklets){

	// brickletIDs are 1-based
	return ( startID <=  endID
		&& startID >=  1
		&& endID   >=  1
		&& startID <= numberOfBricklets
		&& endID   <= numberOfBricklets );
}

/*
	A valid brickletID is a positive and non-zero number 
*/
bool isValidBrickletID(int brickletID){
	return (brickletID > 0);
}

/*
	The trace direction knows four different states defined in constants.h.
	In accordance with vernissage the range is from TRACE_UP(=0) to RE_TRACE_DOWN(3)
*/
bool isValidTraceDir(int traceDir){
	return (traceDir >= TRACE_UP && traceDir <= RE_TRACE_DOWN);
}
