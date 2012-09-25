#include <iostream>
#include <string>

#include <AlpinoCorpus/tr1wrap/memory.hh>

#include <boost/scoped_ptr.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>
#include <config.hh>

#include <AlpinoCorpus/macros.hh>

#include <ProgramOptions.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;
using alpinocorpus::Entry;

namespace tr1 = std::tr1;

void listCorpus(tr1::shared_ptr<CorpusReader> reader,
  std::string const &query)
{
  CorpusReader::EntryIterator i;
  
  i = reader->query(CorpusReader::XQUERY, query);

  while (i.hasNext())
  {
    Entry entry = i.next(*reader);
    std::cout << entry.contents << std::endl;
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
      "  -m filename\tLoad macro file" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl <<
      "  -r\t\tProcess a directory of corpora recursively" << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
  boost::scoped_ptr<ProgramOptions> opts;
  try {
    opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
      "m:q:r"));
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  
  if (opts->arguments().size() == 0)
  {
    usage(opts->programName());
    return 1;
  }

  if (!opts->option('q')) {
    std::cerr << opts->programName() <<
      ": you should provide a query with -q." <<
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

    if (!reader->isValidQuery(CorpusReader::XQUERY, false, query)) {
      std::cerr << "Invalid (or unwanted) query: " << query << std::endl;
      return 1;
    }
  }
  
  try {
      listCorpus(reader, query);
  } catch (std::runtime_error const &e) {
      std::cerr << opts->programName() <<
      ": error listing treebank: " << e.what() << std::endl;
      return 1;
  }    
  
  return 0;
}
