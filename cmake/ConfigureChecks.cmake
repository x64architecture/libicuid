include (CheckFunctionExists)

CHECK_FUNCTION_EXISTS(strlcpy      HAVE_STRLCPY)
CHECK_FUNCTION_EXISTS(snprintf     HAVE_SNPRINTF)

if (HAVE_STRLCPY)
    add_definitions(-DHAVE_STRLCPY)
endif()
if (HAVE_SNPRINTF)
    add_definitions(-DHAVE_SNPRINTF)
endif()

