#ifndef ALPINOCORPUS_MACROS_HH
#define ALPINOCORPUS_MACROS_HH

#include <map>
#include <string>

#include <AlpinoCorpus/DLLDefines.hh>

namespace alpinocorpus {

typedef std::map<std::string, std::string> Macros;

ALPINO_CORPUS_EXPORT std::string expandMacros(Macros const &macros, std::string query);
ALPINO_CORPUS_EXPORT Macros loadMacros(std::string const &filename);

}

#endif // ALPINOCORPUS_MACROS_HH
