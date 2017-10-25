#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include <boost/unordered_set.hpp>

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

template<typename T>
std::set<T> unique_to_first(std::set<T> const &a, std::set<T> const &b)
{
  std::set<T> result;
  std::set_difference(a.begin(), a.end(), b.begin(), b.end(),
      std::inserter(result, result.begin()));
  return result;
}

void output_depth_color(size_t depth) {
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

void listCorpus(boost::shared_ptr<CorpusReader> reader,
  std::string const &query,
  bool bracketed,
  bool colorBrackets,
  std::string const &attribute,
  CorpusInfo const &corpusInfo)
{
  CorpusReader::EntryIterator i;
  
  if (query.empty())
    i = reader->entries();
  else
    i = reader->query(CorpusReader::XPATH, query);

  NotEqualsPrevious<std::string> pred;

  boost::unordered_set<std::string> seen;
  while (i.hasNext())
  {
    Entry entry = i.next(*reader);
    if (seen.find(entry.name) == seen.end()) {
      std::cout << entry.name;

      if (bracketed) {
        std::cout << " ";

        std::vector<LexItem> items = reader->sentence(entry.name, query,
            attribute, "_missing_", corpusInfo);

        std::set<size_t> prevMatches = std::set<size_t>();
        for (std::vector<LexItem>::const_iterator itemIter = items.begin();
          itemIter != items.end(); ++itemIter)
        {
          size_t depth = itemIter->matches.size();

          // Find the set of matches starting before the current word.
          std::set<size_t> startAtCurrent = unique_to_first(itemIter->matches,
              prevMatches);

          if (colorBrackets) {
            if (startAtCurrent.size() != 0) {
              output_depth_color(itemIter->matches.size());
            }
          } else {
            for (std::set<size_t>::const_iterator iter = startAtCurrent.begin();
                iter != startAtCurrent.end(); ++iter) {
                std::cout << "[ ";
            }
          }

          std::cout << itemIter->word;

          // Find the set of matches ending after the current word.
          std::vector<LexItem>::const_iterator next = itemIter + 1;
          std::set<size_t> endAtCurrent = itemIter->matches;
          if (next != items.end()) {
            endAtCurrent = unique_to_first(itemIter->matches, next->matches);
          }

          if (colorBrackets) {
            if (next != items.end() && endAtCurrent.size() != 0) {
              std::cout << "\033[0;22m";
            } 
          } else {
            for (std::set<size_t>::const_iterator iter = endAtCurrent.begin();
                iter != endAtCurrent.end(); ++iter)
              std::cout << " ]";
          }

          std::cout << " ";

          prevMatches = itemIter->matches;
        }
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
      "  -c\t\tUse colored bracketing" << std::endl <<
      "  -m filename\tLoad macro file" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl <<
      "  -s\t\tInclude a bracketed sentence" << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
  boost::scoped_ptr<ProgramOptions> opts;
  try {
    opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
      "a:cm:q:s"));
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
      listCorpus(reader, query, opts->option('s'), opts->option('c'), attr, corpusInfo);
  } catch (std::runtime_error const &e) {
      std::cerr << opts->programName() <<
      ": error listing treebank: " << e.what() << std::endl;
      return 1;
  }    
  
  return 0;
}
