project(Tests)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.8.1
        UPDATE_DISCONNECTED 1
)

FetchContent_MakeAvailable(googletest)

set(ALL_TEST)

macro(set_test TESTNAME)
    #data_directory(${TESTNAME} Tests/Data)
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    set_property(TARGET ${TESTNAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${TESTNAME}>)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
    show_compilation_warnings(${TESTNAME})
    set(ALL_TEST ${ALL_TEST} ${ARGN})
    message(=== ${ALL_TEST})
endmacro()

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    set_test(${TESTNAME} ${ARGN})
endmacro()

macro(package_add_test_with_parser TESTNAME)
    assemble_parser(../App/Logic/Parser/)
    add_executable(${TESTNAME} ${ARGN} ${FLEX_SCANNER_OUTPUTS})
    set_test(${TESTNAME} ${ARGN})
endmacro()

package_add_test(first_test FirstTest.cpp)

package_add_test_with_parser(ALL_TESTS ${ALL_TEST})
