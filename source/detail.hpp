/* Copyright (c) 2014 Fabian Schuiki */
#pragma once
#include <boost/filesystem.hpp>

namespace aurisradio {

/// Returns the path to the current user's home directory.
inline boost::filesystem::path home() { return getenv("HOME"); }

} // namespace aurium