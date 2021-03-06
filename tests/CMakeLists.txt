include_directories (${PROJECT_SOURCE_DIR}/include)

set(AMDTDIR "${PROJECT_SOURCE_DIR}/tests/data/amd")
set(INTELTDIR "${PROJECT_SOURCE_DIR}/tests/data/intel")

add_executable(
    icuid_test

    icuid_test.c
)
target_link_libraries(icuid_test icuid)

add_custom_target(generatetest COMMAND ./icuid_test --generate_test cpu.test DEPENDS icuid_test)

add_test(i7-4790K ./icuid_test --run_test ${INTELTDIR}/haswell/i7-4790K.test)
add_test(i7-4770R ./icuid_test --run_test ${INTELTDIR}/haswell/i7-4770R.test)
add_test(i7-4650U ./icuid_test --run_test ${INTELTDIR}/haswell/i7-4650U.test)
add_test(i5-4200M ./icuid_test --run_test ${INTELTDIR}/haswell/i5-4200M.test)
add_test(i5-3570K ./icuid_test --run_test ${INTELTDIR}/ivybridge/i5-3570K.test)
add_test(i5-2500K ./icuid_test --run_test ${INTELTDIR}/sandybridge/i5-2500K.test)
add_test(e3-1245 ./icuid_test --run_test ${INTELTDIR}/sandybridge/e3-1245.test)
add_test(e7500 ./icuid_test --run_test ${INTELTDIR}/wolfdale/e7500.test)
add_test(ryzen-3500u ./icuid_test --run_test ${AMDTDIR}/zen+/ryzen-3500u.test)
add_test(J4125 ./icuid_test --run_test ${INTELTDIR}/geminilake/J4125.test)