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
    target_link_libraries(${TESTNAME} gtest gmock gtest_main OurSQL_Client_SURSE)
    set_property(TARGET ${TESTNAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${TESTNAME}>)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
    show_compilation_warnings(${TESTNAME})
    set(ALL_TEST ${ALL_TEST} ${ARGN})
endmacro()

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN} Test.h Test.cpp RequestGenerator.h RequestGenerator.cpp)
    package_set_test(${TESTNAME} ${ARGN})
endmacro()

init(${PROJECT_BINARY_DIR})

package_add_test(FullTests FullTests.cpp)
