cmake_minimum_required(VERSION 3.5)

# Library project
project(atcmd_lib LANGUAGES CXX)

# Set project version
set(PROJECT_VERSION "0.0.1")

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library sources
set(LIB_SOURCES
    src/characters.cpp
    src/server/sparameters.cpp
    src/server/server_base.cpp
    src/server/command_base.cpp
    src/server/extendedcommand.cpp
    src/server/extcmddef.cpp
    src/server/basiccmddef.cpp
    src/server/basiccommand.cpp
)

# Library headers (public interface)
set(LIB_HEADERS
    include/atcmd/common.h
    include/atcmd/server/server.h
    include/atcmd/server/sparameters.h
    include/atcmd/server/extendedcommand.h
    include/atcmd/server/command_base.h
    include/atcmd/server/basiccommand.h
    include/atcmd/server/server_base.h
    include/atcmd/detail/basiccmddef.h
    include/atcmd/detail/characters.h
    include/atcmd/detail/cmdparamdef.h
    include/atcmd/detail/extcmddef.h
    include/atcmd/detail/server_cmdline.h
    include/atcmd/detail/triebuilder.h
    include/atcmd/detail/trie.h
)

# Create the library
add_library(atcmd ${LIB_SOURCES} ${LIB_HEADERS})

# Set library properties
target_include_directories(atcmd
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Install targets
include(GNUInstallDirs)

install(TARGETS atcmd
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
