#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/tr1/memory.hpp>
#include <boost/tr1/unordered_set.hpp>

#include <boost/scoped_ptr.hpp>

extern "C" {
#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>
}

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/macros.hh>

#include "../src/util/textfile.hh" // XXX - hmpf

#include <EqualsPrevious.hh>
#include <ProgramOptions.hh>
#include <Stylesheet.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;

namespace tr1 = std::tr1;

void transformCorpus(tr1::shared_ptr<CorpusReader> reader,
  std::string const &query, std::string const &stylesheet)
{
    std::list<CorpusReader::MarkerQuery> markerQueries;
    if (!query.empty()) {
        // Markers
        CorpusReader::MarkerQuery activeMarker(query, "active", "1");
        markerQueries.push_back(activeMarker); 
    }

    CorpusReader::EntryIterator i;
    
    if (!query.empty())
        i = reader->queryWithStylesheet(CorpusReader::XPATH, query,
            stylesheet, markerQueries);
    else
        i = reader->entriesWithStylesheet(stylesheet);

    tr1::unordered_set<std::string> seen;

    while (i.hasNext())
    {
        alpinocorpus::Entry e = i.next(*reader);
        if (seen.find(e.name) == seen.end())
        {
            seen.insert(e.name);
            std::cout << e.contents;
        }
    }
}

void transformEntry(tr1::shared_ptr<CorpusReader> reader,
  std::string const &query,
  std::string const &stylesheet,
  std::string const &entry)
{
  Stylesheet compiledStylesheet(stylesheet);

  std::list<CorpusReader::MarkerQuery> markerQueries;
  if (!query.empty()) {
     // Markers
    CorpusReader::MarkerQuery activeMarker(query, "active", "1");
    markerQueries.push_back(activeMarker);
  }
  std::cout << compiledStylesheet.transform(reader->read(entry, markerQueries));
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] stylesheet treebanks" <<
      std::endl << std::endl <<
      "  -g entry\tApply the stylesheet to a single entry" << std::endl <<
      "  -m filename\tLoad macro file" << std::endl <<
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
      "g:m:q:r"));
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

  alpinocorpus::Macros macros;
  if (opts->option('m')) {
    std::string macrosFn = opts->optionValue('m');
    try {
      macros = alpinocorpus::loadMacros(macrosFn);
    } catch (std::runtime_error &e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }
  }

  std::string query;
  if (opts->option('q')) {
    query = alpinocorpus::expandMacros(macros, opts->optionValue('q'));

    if (!reader->isValidQuery(CorpusReader::XPATH, false, query)) {
      std::cerr << "Invalid (or unwanted) query: " << query << std::endl;
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
