#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/Stylesheet.hh>
#include <AlpinoCorpus/macros.hh>

#include "../src/util/textfile.hh" // XXX - hmpf

#include <ProgramOptions.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;
using alpinocorpus::Either;

void transformCorpus(std::shared_ptr<CorpusReader> reader,
  std::string const &query, std::string const &stylesheetFilename)
{
    std::list<CorpusReader::MarkerQuery> markerQueries;
    if (!query.empty()) {
        // Markers
        CorpusReader::MarkerQuery activeMarker(query, "active", "1");
        markerQueries.push_back(activeMarker); 
    }

    CorpusReader::EntryIterator i;

    std::unique_ptr<alpinocorpus::Stylesheet> parsedStylesheet(alpinocorpus::Stylesheet::readFile(stylesheetFilename));

    if (!query.empty())
        i = reader->queryWithStylesheet(CorpusReader::XPATH, query,
            *parsedStylesheet, markerQueries);
    else
        i = reader->entriesWithStylesheet(*parsedStylesheet);

    std::unordered_set<std::string> seen;

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

void transformEntry(std::shared_ptr<CorpusReader> reader,
  std::string const &query,
  std::string const &stylesheetFilename,
  std::string const &entry)
{
  std::unique_ptr<alpinocorpus::Stylesheet> parsedStylesheet(alpinocorpus::Stylesheet::readFile(stylesheetFilename));

  std::list<CorpusReader::MarkerQuery> markerQueries;
  if (!query.empty()) {
     // Markers
    CorpusReader::MarkerQuery activeMarker(query, "active", "1");
    markerQueries.push_back(activeMarker);
  }
  std::cout << parsedStylesheet->transform(reader->read(entry, markerQueries));
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] stylesheet treebanks" <<
      std::endl << std::endl <<
      "  -g entry\tApply the stylesheet to a single entry" << std::endl <<
      "  -m filename\tLoad macro file" << std::endl <<
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

  std::unique_ptr<ProgramOptions> opts;
  try {
    opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
      "g:m:q:"));
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (opts->arguments().size() < 2)
  {
    usage(opts->programName());
    return 1;
  }

  std::string stylesheetFilename = opts->arguments().at(0);

  std::shared_ptr<CorpusReader> reader;
  try {
    if (opts->arguments().size() == 2)
      reader = std::shared_ptr<CorpusReader>(
        openCorpus(opts->arguments().at(1), true));
    else
      reader = openCorpora(opts->arguments().begin() + 1, 
            opts->arguments().end(), true);
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

    Either<std::string, alpinocorpus::Empty> valid =
      reader->isValidQuery(CorpusReader::XPATH, false, query);
    if (valid.isLeft()) {
      std::cerr << "Invalid (or unwanted) query: " << query << std::endl << std::endl;
      std::cerr << valid.left() << std::endl;
      return 1;
    }
  }

  try {
    if (opts->option('g'))
      transformEntry(reader, query, stylesheetFilename, opts->optionValue('g'));
    else
      transformCorpus(reader, query, stylesheetFilename);
  } catch (std::runtime_error &e) {
    std::cerr << "Error while transforming corpus: " << e.what() << std::endl;
    return 1;
  }
}
