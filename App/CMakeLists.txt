project(App)

set(HEADERS
        )

set(DEFINITIONS
        main.cpp
        )

add_executable(${PROJECT_NAME} ${HEADERS} ${DEFINITIONS})

show_compilation_warnings(${PROJECT_NAME})
