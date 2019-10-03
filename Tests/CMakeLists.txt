project(Tests)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.8.1
        UPDATE_DISCONNECTED 1
)
FetchContent_MakeAvailable(googletest)

macro(package_set_test TESTNAME)
    #data_directory(${TESTNAME} Tests/Data)
    target_link_libraries(${TESTNAME} gtest gmock gtest_main OurSql_APP_SURSE)
    set_property(TARGET ${TESTNAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${TESTNAME}>)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
    show_compilation_warnings(${TESTNAME})
    set(ALL_TEST ${ALL_TEST} ${ARGN})
endmacro()

macro(package_add_test_with_parser TESTNAME)
    add_executable(${TESTNAME} ${ARGN} ${FLEX_SCANNER_OUTPUTS} Test.h Test.cpp)
    package_set_test(${TESTNAME} ${ARGN})
endmacro()

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    package_set_test(${TESTNAME} ${ARGN})
endmacro()

assemble_parser(../App/Logic/Parser/)

package_add_test_with_parser(Logic Logic/ParserTests.cpp)
package_add_test(Engine Engine/EngineTests.cpp)

package_add_test_with_parser(AutoGen AutoGenTests.cpp)
