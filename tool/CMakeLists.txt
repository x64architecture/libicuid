include_directories (${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include)

if (MSVC)
    set(RC "icuidtool.rc")
endif()

add_executable(
    icuid_tool

    ${RC}
    icuid_tool.c
    opt.c
)
target_link_libraries(icuid_tool icuid)

install(TARGETS icuid_tool
        RUNTIME DESTINATION bin)
