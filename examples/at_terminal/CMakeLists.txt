# Example sources
set(EXAMPLE_SOURCES
    main.cpp
    server_config.cpp server_config.h
    asyncioemulator.cpp asyncioemulator.h
    asyncinput.cpp asyncinput.h
    asyncworker.cpp asyncworker.h
    commands/basic/i.cpp commands/basic/i.h
    commands/basic/v.cpp commands/basic/v.h
    commands/extended/gci.cpp commands/extended/gci.h
    commands/extended/gmm.cpp commands/extended/gmm.h
    commands/extended/gmi.cpp commands/extended/gmi.h
    commands/extended/mv18am.cpp commands/extended/mv18am.h
    commands/extended/test1dhb.cpp commands/extended/test1dhb.h
    commands/extended/test2sds.cpp commands/extended/test2sds.h
    commands/extended/test3rsr.cpp commands/extended/test3rsr.h
    commands/extended/test4async.cpp commands/extended/test4async.h
)

# Create the executable
add_executable(at_terminal ${EXAMPLE_SOURCES})

# Set target properties
set_target_properties(at_terminal PROPERTIES CXX_EXTENSIONS OFF)

# Stricter compiller warnings
target_compile_options(at_terminal
    PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall;-Wextra;-Wpedantic>
      $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Link with the library
target_link_libraries(at_terminal PRIVATE atcmd::atcmd)

# Installation
include(GNUInstallDirs)
install(TARGETS at_terminal
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
