#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include <boost/tr1/memory.hpp>
#include <tr1/unordered_set>

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>
#include <config.hh>

#if defined(USE_DBXML)
  #include <AlpinoCorpus/DbCorpusWriter.hh>
#endif

#include <AlpinoCorpus/CompactCorpusWriter.hh>
#include <AlpinoCorpus/LexItem.hh>

#include <iostream>
#include <stdexcept>

#include <EqualsPrevious.hh>
#include <ProgramOptions.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;
using alpinocorpus::CorpusWriter;
using alpinocorpus::CompactCorpusWriter;
using alpinocorpus::Entry;
using alpinocorpus::LexItem;

#if defined(USE_DBXML)
using alpinocorpus::DbCorpusWriter;
#endif

namespace bf = boost::filesystem;
namespace tr1 = std::tr1;

void listCorpus(tr1::shared_ptr<CorpusReader> reader,
  std::string const &query, bool bracketed = false)
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

        std::vector<LexItem> items = reader->sentence(entry.name, query);

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

void readEntry(tr1::shared_ptr<CorpusReader> reader, std::string const &entry)
{
  std::cout << reader->read(entry);
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] treebanks" <<
      std::endl << std::endl <<
      "  -c filename\tCreate a compact corpus archive" << std::endl <<
#if defined(USE_DBXML)
      "  -d filename\tCreate a Dact dbxml archive" << std::endl <<
#endif
      "  -g entry\tPrint a treebank entry to stdout" << std::endl <<
      "  -l\t\tList the entries of a treebank" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl <<
      "  -s\t\tInclude a bracketed sentence" << std::endl <<
      "  -r\t\tProcess a directory of corpora recursively" << std::endl << std::endl;
}

void writeCorpus(tr1::shared_ptr<CorpusReader> reader,
  tr1::shared_ptr<CorpusWriter> writer,
  std::string const &query)
{
  CorpusReader::EntryIterator i;
  if (query.empty())
    i = reader->entries();
  else
    i = reader->query(CorpusReader::XPATH, query);
  
  // We need to be *really* sure when writing a corpus that an entry was not written
  // before. So, we'll use a set, rather than a basic filter.
  tr1::unordered_set<std::string> seen;
  while (i.hasNext()) {
    Entry e = i.next(*reader);

    if (seen.find(e.name) == seen.end()) {
        writer->write(e.name, reader->read(e.name));
        seen.insert(e.name);
    } else
      std::cerr << "Duplicate entry: " << e.name << std::endl;
  }
}

int main(int argc, char *argv[])
{
  boost::scoped_ptr<ProgramOptions> opts;
  try {
    opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
      "c:d:g:lq:rs"));
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  
  if (opts->arguments().size() == 0)
  {
    usage(opts->programName());
    return 1;
  }

  size_t cmdCount = 0;
  char const commands[] = "cdgl";
  for (size_t i = 0; i < sizeof(commands); ++i)
    if (opts->option(commands[i]))
      ++cmdCount;
  
  if (cmdCount > 1) {
    std::cerr << opts->programName() <<
      ": the '-c', 'd', '-g', and '-l' options cannot be used simultaneously." <<
      std::endl;
    return 1;
  }
  
  if (cmdCount == 0) {
    std::cerr << opts->programName() <<
    ": one of the '-c', 'd', -g' or '-l' option should be used." <<
    std::endl;
    return 1;
  }
 
  tr1::shared_ptr<CorpusReader> reader;
  try {
    if (opts->arguments().size() == 1)
      reader = tr1::shared_ptr<CorpusReader>(
        openCorpus(opts->arguments().at(0), opts->option('r')));
    else
      reader = tr1::shared_ptr<CorpusReader>(
        openCorpora(opts->arguments().begin(),
          opts->arguments().end(), opts->option('r')));
  } catch (std::runtime_error &e) {
    std::cerr << "Could not open corpus: " << e.what() << std::endl;
    return 1;
  }
  
  std::string query;
  if (opts->option('q')) {
    query = opts->optionValue('q');  

    if (!reader->isValidQuery(CorpusReader::XPATH, false, query)) {
      std::cerr << "Invalid (or unwanted) query: " << query << std::endl;
      return 1;
    }
  }
  
  if (opts->option('d')) {
    try {
        std::string treebankOut = opts->optionValue('d').c_str();

        // XXX - needs a more sophisticated check now, the output treebank
        // could also be in the search path of a recursive reader.
        for (std::vector<std::string>::const_iterator iter =
            opts->arguments().begin(); iter != opts->arguments().end();
            ++iter)
          if (bf::equivalent(treebankOut, *iter))
            throw std::runtime_error("Attempting to write to the source treebank.");
  
#if defined(USE_DBXML)
        tr1::shared_ptr<CorpusWriter> wr(new DbCorpusWriter(treebankOut, true));
        writeCorpus(reader, wr, query);
#else
        throw std::runtime_error("AlpinoCorpus was compiled without DBXML support.");
#endif // defined(USE_DBXML)

    } catch (std::runtime_error const &e) {
        std::cerr << opts->programName() <<
        ": error creating Dact treebank: " << e.what() << std::endl;
        return 1;
    }
  }
  else if (opts->option('c')) {
    try {
        std::string treebankOut = opts->optionValue('c').c_str();

        // XXX - needs a more sophisticated check now, the output treebank
        // could also be in the search path of a recursive reader.
        for (std::vector<std::string>::const_iterator iter =
            opts->arguments().begin(); iter != opts->arguments().end();
            ++iter)
          if (bf::equivalent(treebankOut, *iter))
            throw std::runtime_error("Attempting to write to the source treebank.");
  
        tr1::shared_ptr<CorpusWriter> wr(new CompactCorpusWriter(treebankOut));
        writeCorpus(reader, wr, query);

    } catch (std::runtime_error const &e) {
        std::cerr << opts->programName() <<
        ": error creating compact corpus: " << e.what() << std::endl;
        return 1;
    }    
  }
  else if (opts->option('g')) {
    try {
      readEntry(reader, opts->optionValue('g'));
    } catch (std::runtime_error const &e) {
        std::cerr << opts->programName() <<
        ": error reading entry: " << e.what() << std::endl;
        return 1;
    }    
  }
  else if (opts->option('l')) {
    try {
        listCorpus(reader, query, opts->option('s'));
    } catch (std::runtime_error const &e) {
        std::cerr << opts->programName() <<
        ": error listing treebank: " << e.what() << std::endl;
        return 1;
    }    
  }
  
  return 0;
}
