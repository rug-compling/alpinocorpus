#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/tr1/memory.hpp>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/scoped_ptr.hpp>

extern "C" {
#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>
}

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include "../src/util/textfile.hh" // XXX - hmpf

#include <EqualsPrevious.hh>
#include <ProgramOptions.hh>
#include <Stylesheet.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;

namespace tr1 = std::tr1;

typedef boost::filter_iterator<NotEqualsPrevious<std::string>, CorpusReader::EntryIterator>
    UniqueFilterIter;

void transformCorpus(tr1::shared_ptr<CorpusReader> reader,
  tr1::shared_ptr<std::string const> query, std::string const &stylesheet)
{
  std::list<CorpusReader::MarkerQuery> markerQueries;
  if (query) {
     // Markers
    CorpusReader::MarkerQuery activeMarker(*query, "active", "1");
    markerQueries.push_back(activeMarker); 
  }

  CorpusReader::EntryIterator i, end(reader->end());
  
  if (query)
    i = reader->queryWithStylesheet(CorpusReader::XPATH, *query,
      stylesheet, markerQueries);
  else
    i = reader->beginWithStylesheet(stylesheet);

  NotEqualsPrevious<std::string> pred;

  for (UniqueFilterIter iter(pred, i, end); iter != UniqueFilterIter(pred, end, end);
      ++iter)
    try {
      std::cout << i.contents(*reader);
    } catch (std::runtime_error &e) {
      std::cerr << "Could not apply stylesheet to: " << *iter << std::endl;
    }
}

void transformEntry(tr1::shared_ptr<CorpusReader> reader,
  tr1::shared_ptr<std::string const> query, std::string stylesheet,
  std::string const &entry)
{
  Stylesheet compiledStylesheet(stylesheet);

  std::list<CorpusReader::MarkerQuery> markerQueries;
  if (query) {
     // Markers
    CorpusReader::MarkerQuery activeMarker(*query, "active", "1");
    markerQueries.push_back(activeMarker);
  }
  std::cout << compiledStylesheet.transform(reader->read(entry, markerQueries));
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] stylesheet treebanks" <<
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

  if (opts->arguments().size() < 2)
  {
    usage(opts->programName());
    return 1;
  }

  std::string stylesheet;
  try {
    stylesheet = alpinocorpus::util::readFile(opts->arguments().at(0));
  } catch (std::runtime_error &e) {
    std::cerr << "Could not read stylesheet: " << e.what() << std::endl;
    return 1;
  }

  tr1::shared_ptr<CorpusReader> reader;
  try {
    if (opts->arguments().size() == 2)
      reader = tr1::shared_ptr<CorpusReader>(
        openCorpus(opts->arguments().at(1), opts->option('r')));
    else
      reader = tr1::shared_ptr<CorpusReader>(
        openCorpora(opts->arguments().begin() + 1, 
            opts->arguments().end(), opts->option('r')));
  } catch (std::runtime_error &e) {
    std::cerr << "Could not open corpus: " << e.what() << std::endl;
    return 1;
  }

  tr1::shared_ptr<std::string> query;
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
