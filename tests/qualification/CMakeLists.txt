configure_file(qualification.sh.in qualification.sh @ONLY)
file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/qualification.sh PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
add_test(NAME fam001-qualification COMMAND ${CMAKE_CURRENT_BINARY_DIR}/qualification.sh)
set_tests_properties(fam001-qualification PROPERTIES TIMEOUT 180)

