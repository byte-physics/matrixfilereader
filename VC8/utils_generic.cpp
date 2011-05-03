/*
	The file utils_generic.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/
#include "header.h"

#include "utils_generic.h"

#include <string>
#include <vector>

#include "globaldata.h"

/*
	Convert a string to a wstring
*/
std::wstring StringToWString(const std::string& s){
	std::wstring temp(s.length(),L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

/*
	Convert a wstring to a string
*/
std::string WStringToString(const std::wstring& s){
	std::string temp(s.length(), ' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

/*
	Convert a char ptr to a string
*/
std::wstring CharPtrToWString(char* cStr){

	if(cStr == NULL){
		return std::wstring();
	}

	std::string str(cStr);
	return StringToWString(str);
}

/*
	Convenience wrapper
*/
void splitString(const char* strChar, const char* sepChar, std::vector<std::string> &list){
	
	list.clear();

	if(strChar == NULL){
		return;
	}

	std::string str = strChar;
	splitString(str, sepChar, list);
}

/*
	Splits string separated by sepChar into a vector of strings
*/
void splitString(const std::string &string, const char* sepChar, std::vector<std::string> &list){

	if(sepChar == NULL){
		return;
	}

	list.clear();
	std::string stringCopy = string;
	int pos=-1;
	int offset=0;

	stringCopy.append(sepChar); // add ; at the end to make the list complete, double ;; are no problem

	sprintf(globDataPtr->outputBuffer,"keyList=%s",stringCopy.c_str());
	debugOutputToHistory(globDataPtr->outputBuffer);

	while( ( pos = stringCopy.find(sepChar,offset) ) != std::string::npos ){

		if(pos == offset){// skip empty element
			offset++;
			continue;
		}

		list.push_back(stringCopy.substr(offset,pos-offset));
		sprintf(globDataPtr->outputBuffer,"key=%s,pos=%d,offset=%d",list.back().c_str(),pos,offset);
		debugOutputToHistory(globDataPtr->outputBuffer);

		offset = pos+1;
	}
}

/*
	Undos a former split string, therefore concatenating each element plus sepchar of list into one string
*/
void joinString(const std::vector<std::string> &list, const char* sepChar, std::string &joinedList){

	joinedList.clear();
	std::vector<std::string>::const_iterator it;
	for(it = list.begin(); it != list.end(); it++){
		joinedList.append(*it);
		joinedList.append(sepChar);	
	}

	return;
}

/*
	Some XOP parameters are of type double, but internally we want them to be bool
	Therefore we want to convert it into a planned way and not by casting
*/
bool doubleToBool(double value){
	
	bool result;

	if( value < DBL_EPSILON){
		result = false;
	}
	else{
		result = true;
	}

	return result;
}
