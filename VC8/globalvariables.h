
#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#define ARRAY_SIZE 1000
#define MAX_DEBUG_LEVEL 10

#include "error-codes.h"

#include "dataclass.h"
extern myData *pMyData;

const char myXopVersion[] = "0.02";

const char VT_SPECIAL_STRING[] = "vt_Special";
const char VT_INTEGER_STRING[] = "vt_Integer";
const char VT_DOUBLE_STRING[]  = "vt_Double";
const char VT_BOOLEAN_STRING[] = "vt_Boolean";
const char VT_ENUM_STRING[]	   = "vt_Enum";
const char VT_STRING_STRING[]  = "vt_String";

#endif // GLOBALVARIABLES_H