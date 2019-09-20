cmake_minimum_required(VERSION 3.14)
project(OurSQL)

set(CMAKE_CXX_STANDARD 17)

macro(show_compilation_warnings TERGET)
    target_compile_options(${TERGET} PRIVATE
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall -Wextra>
            $<$<CXX_COMPILER_ID:MSVC>:
            /W4>
            )
endmacro()

macro(data_directory TARGET DIRECTORY)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND}
            -E copy_directory ${CMAKE_SOURCE_DIR}/${DIRECTORY}/
            ${CMAKE_BINARY_DIR}/${DIRECTORY}/)
endmacro()

add_subdirectory(App)

add_subdirectory(Tests)
