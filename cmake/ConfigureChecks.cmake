include (CheckFunctionExists)
include (CheckSymbolExists)

CHECK_FUNCTION_EXISTS(strlcpy           HAVE_STRLCPY)
CHECK_SYMBOL_EXISTS(snprintf "stdio.h"  HAVE_SNPRINTF)

if (HAVE_STRLCPY)
    add_definitions(-DHAVE_STRLCPY)
endif()
if (HAVE_SNPRINTF)
    add_definitions(-DHAVE_SNPRINTF)
endif()

