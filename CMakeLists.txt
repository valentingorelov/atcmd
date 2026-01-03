cmake_minimum_required(VERSION 3.5)

project(atcmd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Optional: Disables GNU extensions for stricter C++23 adherence

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
	string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
	if(flag_already_set EQUAL -1)
		check_cxx_compiler_flag("${flag}" flag_supported)
		if(flag_supported)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
		endif()
		unset(flag_supported CACHE)
	endif()
endfunction()

enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")

include_directories(lib/include)

add_executable(at_terminal
	lib/include/atcmd/detail/characters.h
	lib/include/atcmd/detail/extcmddef.h
	lib/include/atcmd/detail/trie.h
	lib/include/atcmd/detail/triebuilder.h

	lib/include/atcmd/server/server.h
	lib/include/atcmd/server/sparameters.h
	lib/src/server/sparameters.cpp
	lib/src/characters.cpp
	lib/include/atcmd/server/extendedcommand.h
	lib/include/atcmd/server/command_base.h
	lib/include/atcmd/common.h
	lib/include/atcmd/server/basiccommand.h
	lib/include/atcmd/server/server_base.h
	lib/src/server/server_base.cpp
	lib/src/server/command_base.cpp
	lib/src/server/extendedcommand.cpp
	lib/src/server/extcmddef.cpp
	lib/include/atcmd/detail/server_cmdline.h

	examples/at_terminal/commands/extended/gci.h examples/at_terminal/commands/extended/gci.cpp
	examples/at_terminal/commands/extended/gmm.h examples/at_terminal/commands/extended/gmm.cpp
	examples/at_terminal/commands/extended/gmi.h examples/at_terminal/commands/extended/gmi.cpp
	examples/at_terminal/main.cpp
	examples/at_terminal/commands/basic/v.h examples/at_terminal/commands/basic/v.cpp
	lib/include/atcmd/detail/basiccmddef.h
	lib/src/server/basiccmddef.cpp
	lib/include/atcmd/detail/cmdparamdef.h
	lib/src/server/basiccommand.cpp
	examples/at_terminal/commands/extended/mv18am.h examples/at_terminal/commands/extended/mv18am.cpp
	examples/at_terminal/commands/extended/test1dhb.h examples/at_terminal/commands/extended/test1dhb.cpp
	examples/at_terminal/commands/extended/test2sds.h examples/at_terminal/commands/extended/test2sds.cpp
	examples/at_terminal/commands/extended/test3rsr.h examples/at_terminal/commands/extended/test3rsr.cpp
	examples/at_terminal/commands/basic/i.h examples/at_terminal/commands/basic/i.cpp

	)

include(GNUInstallDirs)

# install(TARGETS AtServer
# 	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# 	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
