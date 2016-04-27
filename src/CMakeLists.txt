include_directories(${PROJECT_SOURCE_DIR}/include)
add_definitions(-DLIBICUID_INTERNAL)

if(MSVC)
    set(WINDOWS_SRC windows/libicuid.rc windows/exports.def)
endif()

add_library(
    icuid

    ${WINDOWS_SRC}
    icuid.c
    features.c
    intel.c
    amd.c
    error.c
    match.c

    $<TARGET_OBJECTS:cc>
    $<TARGET_OBJECTS:compat>
)
set_target_properties(icuid PROPERTIES PREFIX "lib")

install(TARGETS icuid
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

add_subdirectory(cc)
add_subdirectory(compat)
