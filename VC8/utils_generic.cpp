/*
	The file utils_generic.cpp is part of the "MatrixFileReader XOP".
	It is licensed under the LGPLv3 with additional permissions,
	see License.txt in the source folder for details.
*/
#include "stdafx.h"

#include "utils_generic.h"
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

	sprintf(GlobalData::Instance().outputBuffer,"keyList=%s",stringCopy.c_str());
	debugOutputToHistory(GlobalData::Instance().outputBuffer);

	while( ( pos = stringCopy.find(sepChar,offset) ) != std::string::npos ){

		if(pos == offset){// skip empty element
			offset++;
			continue;
		}

		list.push_back(stringCopy.substr(offset,pos-offset));
		sprintf(GlobalData::Instance().outputBuffer,"key=%s,pos=%d,offset=%d",list.back().c_str(),pos,offset);
		debugOutputToHistory(GlobalData::Instance().outputBuffer);

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
}

// still no unit tests :(
//char str1[] = "a\\";
//RemoveAllBackslashesAtTheEnd(str1);
//std::cout << "str1 _" << str1 << "_" << std::endl;
//
//char* str2 = NULL;
//RemoveAllBackslashesAtTheEnd(str2);
//std::cout << "str2 _" << "(null)" << "_" << std::endl;
//
//char str3[] = "\\ab\\\\";
//RemoveAllBackslashesAtTheEnd(str3);
//std::cout << "str3 _" << str3 << "_" << std::endl;
//
//char str4[] = "\\ab\\\\cd\\";
//RemoveAllBackslashesAtTheEnd(str4);
//std::cout << "str4 _" << str4 << "_" << std::endl;
//
//char str5[] = "";
//RemoveAllBackslashesAtTheEnd(str5);
//std::cout << "str5 _" << str5 << "_" << std::endl;
//
//char str6[] = "\\\\\\";
//RemoveAllBackslashesAtTheEnd(str6);
//std::cout << "str6 _" << str6 << "_" << std::endl;

// Remove all backslashes at the end of the string
void RemoveAllBackslashesAtTheEnd( char* str ) 
{
	if (str == NULL || strlen(str) == 0 )
	{
		return;
	}

	size_t i;
	for (i = strlen(str); i > 0 && str[i-1] == '\\'; i--)
	{ /* do nothing */ }

	str[i] = '\0';
}

/*
	Some XOP parameters are of type double, but internally we want them to be bool
	Therefore we want to convert it into a planned way and not by casting
*/
bool doubleToBool(double value){
	
	return (value < DBL_EPSILON ? false : true);
}
