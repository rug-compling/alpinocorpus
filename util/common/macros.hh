#include <string>

#include "parseMacros.hh"


std::string expandMacros(Macros const &macros, std::string query);
Macros loadMacros(std::string const &filename);
