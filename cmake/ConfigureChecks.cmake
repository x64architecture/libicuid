include (CheckFunctionExists)

CHECK_FUNCTION_EXISTS(strlcpy HAVE_STRLCPY)

if (HAVE_STRLCPY)
    add_definitions(-DHAVE_STRLCPY)
endif()
