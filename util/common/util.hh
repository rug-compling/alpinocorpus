#ifndef ALPINOCORPUS_COMMON_UTIL_HH
#define ALPINOCORPUS_COMMON_UTIL_HH

#include <string>
#include <vector>

#include <AlpinoCorpus/CorpusReader.hh>

alpinocorpus::CorpusReader *openCorpus(std::string const &path,
    bool recursive);

alpinocorpus::CorpusReader *openCorpora(
    std::vector<std::string>::const_iterator const &pathBegin,    
    std::vector<std::string>::const_iterator const &pathEnd,
    bool recursive);

#endif // ALPINOCORPUS_COMMON_UTIL_HH
