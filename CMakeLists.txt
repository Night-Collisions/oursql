cmake_minimum_required(VERSION 3.14)
project(OurSQL)

set(CMAKE_CXX_STANDARD 17)

macro(show_compilation_warnings TERGET)
    target_compile_options(${TERGET} PRIVATE
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall -Wextra -DDEBUG>

            $<$<CXX_COMPILER_ID:MSVC>:
            /W4>
            )
endmacro()

macro(data_directory TARGET DIRECTORY)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND}
            -E copy_directory ${CMAKE_SOURCE_DIR}/${DIRECTORY}/
            ${CMAKE_BINARY_DIR}/${DIRECTORY}/)
endmacro()

find_package(FLEX)
find_package(BISON)

macro(assemble_parser WAY)
FLEX_TARGET(SCANNER ${CMAKE_CURRENT_SOURCE_DIR}/${WAY}scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
BISON_TARGET(PARSER ${CMAKE_CURRENT_SOURCE_DIR}/${WAY}parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp COMPILE_FLAGS -t)
ADD_FLEX_BISON_DEPENDENCY(SCANNER PARSER)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
endmacro()

macro(init WAY)
    file(MAKE_DIRECTORY ${WAY}/DataBD)
endmacro()

include(FetchContent)

FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio
        GIT_TAG asio-1-14-0
        UPDATE_DISCONNECTED 1
)

SET(CMAKE_CXX_FLAGS "-DASIO_STANDALONE -pthread")
FetchContent_Populate(asio)
include_directories(${asio_SOURCE_DIR}/asio/include/)

if(WIN32)
    link_libraries(ws2_32 wsock32)
endif()

add_subdirectory(Server)

add_subdirectory(Client)

add_subdirectory(Tests)
