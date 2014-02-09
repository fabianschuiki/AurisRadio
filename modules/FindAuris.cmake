# Copyright (c) 2014 Fabian Schuiki
# Looks for the Auris header-only library. If found, this will define
# - Auris_FOUND
# - AURIS_INCLUDE_DIRS

include(LibFindMacros)

find_path(Auris_INCLUDE_DIR
	NAMES auris/auris.hpp
	PATH_SUFFIXES include
	PATHS $ENV{AURISDIR})

set(Auris_PROCESS_INCLUDES Auris_INCLUDE_DIR)

libfind_process(Auris)
