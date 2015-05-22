include_directories(${CMAKE_CURRENT_BINARY_DIR} ../include)

if (CMAKE_COMPILER_IS_GNUCC OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    set(
        CC_SOURCES

        gcc.c
    )
elseif(MSVC)
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    # cmake reports AMD64 on Windows, but we might be building for 32-bit.
    if (CMAKE_CL_64)
        enable_language(ASM_MASM)
        set(
            CC_SOURCES

            masm-x86_64.asm
        )
    endif()
endif()
    set(
        CC_SOURCES

        ${CC_SOURCES}
        visualc.c
    )
else()
    message(FATAL_ERROR "Unsupported compiler:" ${CMAKE_C_COMPILER})
endif()

add_library(
    cc

    OBJECT

    ${CC_SOURCES}
)
