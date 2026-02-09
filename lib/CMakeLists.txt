cmake_minimum_required(VERSION 3.28.0)

project(atcmd_lib
  VERSION 0.0.1
  LANGUAGES CXX)

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
# Provide an alias target in the build-tree and for consumers use atcmd::atcmd
add_library(atcmd::atcmd ALIAS atcmd)

# Set library properties
target_compile_features(atcmd PUBLIC cxx_std_23)
set_target_properties(atcmd PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(atcmd
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Stricter compiller warnings
target_compile_options(atcmd
    PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall;-Wextra;-Wpedantic>
      $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Installation: install target and headers, and export the target for find_package()
include(GNUInstallDirs)

# Install library and create a namespaced export (atcmd::)
install(TARGETS atcmd
    EXPORT atcmdTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Export the targets to a file for use with find_package
install(EXPORT atcmdTargets
    FILE atcmdTargets.cmake
    NAMESPACE atcmd::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/atcmd
)

# Generate and install Config and ConfigVersion files
include(CMakePackageConfigHelpers)

# Configure a simple atcmdConfig.cmake which includes the installed targets file
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/atcmd")

# Create a minimal config file for package consumers (the .in file will include the installed export)
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/atcmdConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/atcmdConfig.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/atcmdConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/atcmdConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/atcmdConfigVersion.cmake
    DESTINATION ${CONFIG_INSTALL_DIR}
)

if(ATCMD_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Turn off optimizations for nicer coverage reports
    target_compile_options(atcmd PRIVATE -O0 -g --coverage)
    target_link_options(atcmd PRIVATE --coverage)
  else()
    message(FATAL_ERROR "Coverage is only supported with GCC/Clang in this setup.")
  endif()
endif()

if(ATCMD_BUILD_DOCUMENTATION)
  find_package(Doxygen REQUIRED)

  set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")

  set(DOXYGEN_INPUT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  # Doxygen expects a space-separated list
  list(JOIN DOXYGEN_INPUT_DIRS " " DOXYGEN_INPUT_DIRS)

  set(DOXYFILE_IN  "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
  set(DOXYFILE_OUT "${CMAKE_BINARY_DIR}/Doxyfile")

  configure_file("${DOXYFILE_IN}" "${DOXYFILE_OUT}" @ONLY)

  add_custom_target(atcmd_doc ALL
    COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE_OUT}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
endif()
