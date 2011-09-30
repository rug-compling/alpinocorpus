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
  if (recursive)
    return CorpusReader::openRecursive(path);
  else
    return CorpusReader::open(path);
}

void transformCorpus(std::tr1::shared_ptr<CorpusReader> reader,
  std::tr1::shared_ptr<std::string const> query, std::tr1::shared_ptr<Stylesheet> stylesheet)
{
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
    std::cout << stylesheet->transform(reader->readMarkQueries(*i, markerQueries));
}

void transformEntry(std::tr1::shared_ptr<CorpusReader> reader,
  std::tr1::shared_ptr<std::string const> query, std::tr1::shared_ptr<Stylesheet> stylesheet,
  std::string const &entry)
{
  std::list<CorpusReader::MarkerQuery> markerQueries;
  if (query) {
     // Markers
    CorpusReader::MarkerQuery activeMarker(*query, "active", "1");
    markerQueries.push_back(activeMarker); 
  }
  std::cout << stylesheet->transform(reader->readMarkQueries(entry, markerQueries));
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] stylesheet treebank" <<
      std::endl << std::endl <<
      "  -g entry\tApply the stylesheet to a single entry" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl <<
      "  -r\t\tProcess a directory of corpora recursively" << std::endl << std::endl;
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
      "g:q:r"));
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (opts->arguments().size() != 2)
  {
    usage(opts->programName());
    return 1;
  }

  std::tr1::shared_ptr<Stylesheet> stylesheet;
  try {
    std::string stylesheetData = alpinocorpus::util::readFile(opts->arguments().at(0));
    stylesheet.reset(new Stylesheet(stylesheetData));
  } catch (std::runtime_error &e) {
    std::cerr << "Could not parse stylesheet: " << e.what() << std::endl;
    return 1;
  }

  std::tr1::shared_ptr<CorpusReader> reader;
  try {
    reader.reset(openCorpus(opts->arguments().at(1), opts->option('r')));
  } catch (std::runtime_error &e) {
    std::cerr << "Could not open corpus: " << e.what() << std::endl;
    return 1;
  }

  std::tr1::shared_ptr<std::string> query;
  if (opts->option('q')) {
    query.reset(new std::string(opts->optionValue('q')));

    if (!reader->isValidQuery(CorpusReader::XPATH, false, *query)) {
      std::cerr << "Invalid (or unwanted) query: " << *query << std::endl;
      return 1;
    }
  }

  try {
    if (opts->option('g'))
      transformEntry(reader, query, stylesheet, opts->optionValue('g'));
    else
      transformCorpus(reader, query, stylesheet);
  } catch (std::runtime_error &e) {
    std::cerr << "Error while transforming corpus: " << e.what() << std::endl;
  }
}