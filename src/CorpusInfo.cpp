#include <map>
#include <string>

#include <boost/assign/list_of.hpp>

#include <AlpinoCorpus/CorpusInfo.hh>

namespace alpinocorpus {

CorpusInfo const ALPINO_CORPUS_INFO(
    boost::assign::list_of("node"),
    "node",
    "word");

CorpusInfo const TUEBA_DZ_CORPUS_INFO(
    boost::assign::list_of("node")("ne")("word"),
    "word",
    "form");

std::map<std::string, CorpusInfo> const PREDEFINED_CORPORA =
    boost::assign::map_list_of
      ("alpino_ds", ALPINO_CORPUS_INFO)
      ("tueba_tree", TUEBA_DZ_CORPUS_INFO);

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

