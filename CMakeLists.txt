cmake_minimum_required(VERSION 2.8.12)
project(libicuid C)

include(CTest)

if (CMAKE_COMPILER_IS_GNUCC OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wextra -g")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2")
elseif(MSVC)
    set(CMAKE_C_FLAGS "/W3 /WX")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} /MT /Ox /O2 /Ob2")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    message(FATAL_ERROR "Unsupported compiler:" ${CMAKE_C_COMPILER})
endif()

if (BUILD_SHARED_LIBS)
    # Because we use OBJECT we have to globaly set
    # CMAKE_POSITION_INDEPENDENT_CODE to TRUE
    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
endif()

install(DIRECTORY include/icuid DESTINATION include)

add_subdirectory(src)
add_subdirectory(tool)
add_subdirectory(tests)
