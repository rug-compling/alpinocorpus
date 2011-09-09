#ifndef MARKQUERIES_HH
#define MARKQUERIES_HH

#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {
std::string markQueries(std::string data,
  std::list<CorpusReader::MarkerQuery> const &queries);
}

#endif // MARKQUERIES_HH
