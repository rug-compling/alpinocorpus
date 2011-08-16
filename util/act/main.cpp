#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include <tr1/unordered_set>

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>

#include <iostream>
#include <stdexcept>

#include "ProgramOptions.hh"

using alpinocorpus::CorpusReader;
using alpinocorpus::DbCorpusWriter;

namespace bf = boost::filesystem;

void listCorpus(std::string const &treebank, std::string const &query)
{
  boost::scoped_ptr<CorpusReader> rd(CorpusReader::open(treebank));
  CorpusReader::EntryIterator i, end(rd->end());
  
  if (query.empty())
    i = rd->begin();
  else
    i = rd->query(CorpusReader::XPATH, query);

  std::copy(i, end, std::ostream_iterator<std::string>(std::cout, "\n"));  
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] treebank" <<
      std::endl << std::endl <<
      "  -c filename\tCreate a Dact dbxml archive" << std::endl <<
      "  -l\t\tList the entries of a treebank" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl << std::endl;  
}

void writeDactCorpus(std::string const &treebank, std::string const &treebankOut,
    std::string const &query)
{
  if (bf::equivalent(treebankOut, treebank))
    throw std::runtime_error("Attempting to write to the source treebank.");
  
  boost::scoped_ptr<CorpusReader> rd(CorpusReader::open(treebank));
    
  DbCorpusWriter wr(treebankOut, true);
  CorpusReader::EntryIterator i, end(rd->end());
  if (query.empty())
    i = rd->begin();
  else
    i = rd->query(CorpusReader::XPATH, query);
  
  std::tr1::unordered_set<std::string> seen;
  for (; i != end; ++i)
    if (seen.find(*i) == seen.end()) {
        wr.write(*i, rd->read(*i));
      seen.insert(*i);
    }
}

int main(int argc, char *argv[])
{
  ProgramOptions opts(argc, const_cast<char const **>(argv), "c:lq:");
  
  if (opts.arguments().size() != 1)
  {
    usage(opts.programName());
    return 1;
  }
  
  if (opts.option('c') && opts.option('l')) {
    std::cerr << opts.programName() <<
      ": the '-c' and '-l' options cannot be used simultaneously." <<
      std::endl;
    return 1;
  }
  
  if (!opts.option('c') && !opts.option('l')) {
    std::cerr << opts.programName() <<
    ": either the '-c' or '-l' option should be used." <<
    std::endl;
    return 1;
  }
  
  std::string query;
  if (opts.option('q'))
    query = opts.optionValue('q');  
  
  if (opts.option('c')) {
    try {
        std::string treebank = opts.arguments().at(0);
        std::string treebankOut = opts.optionValue('c').c_str();
      writeDactCorpus(treebank, treebankOut, query);
    } catch (std::runtime_error const &e) {
        std::cerr << opts.programName() <<
        ": error creating Dact treebank: " << e.what() << std::endl;
        return 1;
    }
  }
  
  if (opts.option('l')) {
    try {
        std::string treebank = opts.arguments().at(0).c_str();
        listCorpus(treebank, query);
    } catch (std::runtime_error const &e) {
        std::cerr << opts.programName() <<
        ": error listing treebank: " << e.what() << std::endl;
        return 1;
    }    
  }
  
  return 0;
}
