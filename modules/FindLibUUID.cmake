# Locate the UUID library headers.
# Copyright (c) 2013 Fabian Schuiki

set(LIBUUIDDIR "" CACHE PATH "libuuid directory (optional)")

find_path(
	LIBUUID_INCLUDE_DIRS uuid.h
	PATH_SUFFIXES include/uuid
	HINTS ${UUIDDIR})
find_library(
	LIBUUID_LIBRARIES
	NAMES uuid
	PATH_SUFFIXES lib lib64
	HINTS ${UUIDDIR})

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(libuuid DEFAULT_MSG LIBUUID_LIBRARIES LIBUUID_INCLUDE_DIRS)

mark_as_advanced(
	LIBUUID_INCLUDE_DIRS
	LIBUUID_LIBRARIES
)