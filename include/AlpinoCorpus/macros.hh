#ifndef ALPINOCORPUS_MACROS_HH
#define ALPINOCORPUS_MACROS_HH

#include <map>
#include <string>

namespace alpinocorpus {

typedef std::map<std::string, std::string> Macros;

std::string expandMacros(Macros const &macros, std::string query);
Macros loadMacros(std::string const &filename);

}

#endif // ALPINOCORPUS_MACROS_HH
