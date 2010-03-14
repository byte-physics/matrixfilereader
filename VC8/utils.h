

#ifndef UTILS_H
#define UTILS_H

std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);


std::wstring CharPtrToWString(char* cStr);

#endif // UTILS_H