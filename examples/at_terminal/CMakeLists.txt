cmake_minimum_required(VERSION 3.5)

# Terminal example project
project(at_terminal LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Example sources
set(EXAMPLE_SOURCES
    main.cpp
    commands/basic/i.cpp commands/basic/i.h
    commands/basic/v.cpp commands/basic/v.h
    commands/extended/gci.cpp commands/extended/gci.h
    commands/extended/gmm.cpp commands/extended/gmm.h
    commands/extended/gmi.cpp commands/extended/gmi.h
    commands/extended/mv18am.cpp commands/extended/mv18am.h
    commands/extended/test1dhb.cpp commands/extended/test1dhb.h
    commands/extended/test2sds.cpp commands/extended/test2sds.h
    commands/extended/test3rsr.cpp commands/extended/test3rsr.h
)

# Create the executable
add_executable(at_terminal ${EXAMPLE_SOURCES})

# Link with the library
target_link_libraries(at_terminal PRIVATE atcmd)

# Install target
include(GNUInstallDirs)

install(TARGETS at_terminal
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
