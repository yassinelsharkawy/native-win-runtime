#pragma once

#ifndef EOF
#define EOF (-1)
#endif

#ifndef _FILE_DEFINED
#define _FILE_DEFINED
typedef struct _iobuf FILE;
#endif

#ifndef _FPOS_T_DEFINED
#define _FPOS_T_DEFINED
typedef __int64 fpos_t;
#endif

#define PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_HARD 1
#include <printf/printf.h>
#undef PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_HARD
