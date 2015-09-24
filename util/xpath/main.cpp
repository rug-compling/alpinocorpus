#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

#include <boost/tr1/unordered_set.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusInfo.hh>
#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>
#include <AlpinoCorpus/util/Either.hh>
#include <config.hh>

#include <AlpinoCorpus/LexItem.hh>
#include <AlpinoCorpus/macros.hh>

#include <EqualsPrevious.hh>
#include <ProgramOptions.hh>
#include <util.hh>

using alpinocorpus::CorpusInfo;
using alpinocorpus::CorpusReader;
using alpinocorpus::Either;
using alpinocorpus::Entry;
using alpinocorpus::LexItem;

namespace bf = boost::filesystem;
namespace tr1 = std::tr1;

void listCorpus(boost::shared_ptr<CorpusReader> reader,
  std::string const &query, bool bracketed,
  std::string const &attribute,
  CorpusInfo const &corpusInfo)
{
  CorpusReader::EntryIterator i;
  
  if (query.empty())
    i = reader->entries();
  else
    i = reader->query(CorpusReader::XPATH, query);

  NotEqualsPrevious<std::string> pred;

  tr1::unordered_set<std::string> seen;
  while (i.hasNext())
  {
    Entry entry = i.next(*reader);
    if (seen.find(entry.name) == seen.end()) {
      std::cout << entry.name;

      if (bracketed) {
        std::cout << " ";

        std::vector<LexItem> items = reader->sentence(entry.name, query,
            attribute, "_missing_", corpusInfo);

        size_t prevDepth = 0;
        for (std::vector<LexItem>::const_iterator itemIter = items.begin();
          itemIter != items.end(); ++itemIter)
        {
          size_t depth = itemIter->matches.size();

          if (depth != prevDepth) {
            if (depth == 0)
              std::cout << "\033[0;22m";
            else if (depth == 1)
              std::cout << "\033[38;5;99m";
            else if (depth == 2)
              std::cout << "\033[38;5;111m";
            else if (depth == 3)
              std::cout << "\033[38;5;123m";
            else if (depth == 4)
              std::cout << "\033[38;5;121m";
            else
              std::cout << "\033[38;5;119m";
          }

          std::cout << itemIter->word;

          std::vector<LexItem>::const_iterator next = itemIter + 1;
          if (next != items.end() && next->matches.size() < depth)
            std::cout << "\033[0;22m";

          std::cout << " ";

          prevDepth = depth;
        }

        std::cout << "\033[0;22m" << std::endl;
      }

      std::cout << std::endl;
      seen.insert(entry.name);
    }
  }
}

void readEntry(boost::shared_ptr<CorpusReader> reader, std::string const &entry)
{
  std::cout << reader->read(entry);
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] treebank(s)" <<
      std::endl << std::endl <<
      "  -a attr\tLexical attribute to show (default: word)" << std::endl <<
      "  -m filename\tLoad macro file" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl <<
      "  -s\t\tInclude a bracketed sentence" << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
  boost::scoped_ptr<ProgramOptions> opts;
  try {
    opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
      "a:m:q:s"));
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  
  if (opts->arguments().size() == 0)
  {
    usage(opts->programName());
    return 1;
  }
 
  boost::shared_ptr<CorpusReader> reader;
  try {
    if (opts->arguments().size() == 1)
      reader = boost::shared_ptr<CorpusReader>(
        openCorpus(opts->arguments().at(0), true));
    else
      reader = boost::shared_ptr<CorpusReader>(
        openCorpora(opts->arguments().begin(),
          opts->arguments().end(), true));
  } catch (std::runtime_error &e) {
    std::cerr << "Could not open corpus: " << e.what() << std::endl;
    return 1;
  }

  CorpusInfo corpusInfo = alpinocorpus::predefinedCorpusOrFallback(reader->type());

  std::string attr = corpusInfo.tokenAttribute();
  if (opts->option('a')) {
      attr = opts->optionValue('a');
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
      listCorpus(reader, query, opts->option('s'), attr, corpusInfo);
  } catch (std::runtime_error const &e) {
      std::cerr << opts->programName() <<
      ": error listing treebank: " << e.what() << std::endl;
      return 1;
  }    
  
  return 0;
}
