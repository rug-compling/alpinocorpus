#include <iostream>
#include <stdexcept>
#include <string>

#include <tr1/memory>

#include <boost/scoped_ptr.hpp>

extern "C" {
#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>
}


#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include <util/textfile.hh>

#include <ProgramOptions.hh>
#include <Stylesheet.hh>

using alpinocorpus::CorpusReader;

CorpusReader* openCorpus(std::string const &path,
    bool recursive)
{
  try {
    if (recursive)
      return CorpusReader::openRecursive(path);
    else
      return CorpusReader::open(path);
  } catch (std::runtime_error &e) {
    std::cerr << "Could not open corpus " << path << ": " << e.what() << std::endl;
    return 0;
  }
}

void transformCorpus(std::tr1::shared_ptr<CorpusReader> reader,
  std::tr1::shared_ptr<std::string const> query, std::string const &stylesheet)
{
  Stylesheet transformer(stylesheet);

  std::list<CorpusReader::MarkerQuery> markerQueries;
  if (query) {
     // Markers
    CorpusReader::MarkerQuery activeMarker(*query, "active", "1");
    markerQueries.push_back(activeMarker); 
  }

  CorpusReader::EntryIterator i, end(reader->end());
  
  if (query)
    i = reader->query(CorpusReader::XPATH, *query);
  else
    i = reader->begin();

  for (; i != end; ++i)
    std::cout << transformer.transform(reader->readMarkQueries(*i, markerQueries));
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] stylesheet treebank" <<
      std::endl << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl << std::endl;
}

int main (int argc, char *argv[])
{
    xmlInitMemory();
    xmlInitParser();

    // EXSLT extensions
    exsltRegisterAll();

    // XPath
    xmlXPathInit();

  boost::scoped_ptr<ProgramOptions> opts;
  try {
    opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
      "q:"));
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (opts->arguments().size() != 2)
  {
    usage(opts->programName());
    return 1;
  }

  std::string stylesheet = alpinocorpus::util::readFile(opts->arguments().at(0));

  std::tr1::shared_ptr<CorpusReader> reader = std::tr1::shared_ptr<CorpusReader>(
    openCorpus(opts->arguments().at(1), opts->option('r')));

  std::tr1::shared_ptr<std::string> query;
  if (opts->option('q')) {
    query.reset(new std::string(opts->optionValue('q')));

    if (!reader->isValidQuery(CorpusReader::XPATH, false, *query)) {
      std::cerr << "Invalid (or unwanted) query: " << *query << std::endl;
      return 1;
    }
  }

  transformCorpus(reader, query, stylesheet);
}