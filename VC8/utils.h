

#ifndef UTILS_H
#define UTILS_H

std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);

#define ASSERT(A,B) { if(A == NULL){ XOPNotice("ASSERT: Pointer " #A " is NULL. You might want to drop the author a note :)\r"); return B; } }
#define ASSERT_RETURN_ZERO(A) { ASSERT(A,0)}
#define ASSERT_RETURN_VOID(A) { ASSERT(A,)}


std::wstring CharPtrToWString(char* cStr);

void debugOutputToHistory(int level, char *str);
void outputToHistory(char *str);


#endif // UTILS_H