

#ifndef UTILS_H
#define UTILS_H

std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);


std::wstring CharPtrToWString(char* cStr);

void debugOutputToHistory(int level, char *str);
void outputToHistory(char *str);


#endif // UTILS_H