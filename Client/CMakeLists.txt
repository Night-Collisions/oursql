project(OurSQL_Client)

set(HEADERS
        Client.h
        )

set(DEFINITIONS
        Client.cpp
        )

add_executable(${PROJECT_NAME} main.cpp ${HEADERS} ${DEFINITIONS})

add_library(${PROJECT_NAME}_SURSE ${HEADERS} ${DEFINITIONS})

show_compilation_warnings(${PROJECT_NAME})
