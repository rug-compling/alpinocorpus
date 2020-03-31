#include <map>
#include <string>

#include <AlpinoCorpus/CorpusInfo.hh>

namespace alpinocorpus {

CorpusInfo const ALPINO_CORPUS_INFO(
    {"node"},
    "node",
    "word");

CorpusInfo const TUEBA_DZ_CORPUS_INFO(
    {"node", "ne", "word"},
    "word",
    "form");

CorpusInfo const CONLLX_CORPUS_INFO(
    {"word"},
    "word",
    "form");

std::map<std::string, CorpusInfo> const PREDEFINED_CORPORA = {
    {"alpino_ds", ALPINO_CORPUS_INFO},
    {"tueba_tree", TUEBA_DZ_CORPUS_INFO},
    {"conllx_ds", CONLLX_CORPUS_INFO}
};

CorpusInfo const FALLBACK_CORPUS_INFO = ALPINO_CORPUS_INFO;


CorpusInfo predefinedCorpusOrFallback(std::string const &type)
{
    std::map<std::string, CorpusInfo>::const_iterator iter =
        alpinocorpus::PREDEFINED_CORPORA.find(type);
    if (iter != alpinocorpus::PREDEFINED_CORPORA.end())
        return iter->second;

    return alpinocorpus::FALLBACK_CORPUS_INFO;
}

}

