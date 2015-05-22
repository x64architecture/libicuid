include_directories(../include)

if (NOT HAVE_STRLCPY)
    set(
        COMPAT_SOURCES

        strlcpy.c
    )
endif()

add_library(
    compat

    OBJECT

    ${COMPAT_SOURCES}
)
