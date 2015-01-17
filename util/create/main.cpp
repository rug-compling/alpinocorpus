#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>


#include <boost/tr1/unordered_set.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>
#include <AlpinoCorpus/util/Either.hh>
#include <config.hh>

#if defined(USE_DBXML)
  #include <AlpinoCorpus/DbCorpusWriter.hh>
#endif

#include <AlpinoCorpus/CompactCorpusWriter.hh>
#include <AlpinoCorpus/LexItem.hh>
#include <AlpinoCorpus/macros.hh>

#include <iostream>
#include <stdexcept>

#include <EqualsPrevious.hh>
#include <ProgramOptions.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;
using alpinocorpus::CorpusWriter;
using alpinocorpus::CompactCorpusWriter;
using alpinocorpus::Either;
using alpinocorpus::Entry;
using alpinocorpus::LexItem;

#if defined(USE_DBXML)
using alpinocorpus::DbCorpusWriter;
#endif

namespace bf = boost::filesystem;
namespace tr1 = std::tr1;

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] treebanks" <<
      std::endl << std::endl <<
      "  -c filename\tCreate a compact corpus archive" << std::endl <<
#if defined(USE_DBXML)
      "  -d filename\tCreate a Dact dbxml archive" << std::endl <<
#endif
      "  -m filename\tLoad macro file" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl <<
      "  -r\t\tProcess a directory of corpora recursively" << std::endl << std::endl;
}

void writeCorpus(boost::shared_ptr<CorpusReader> reader,
  boost::shared_ptr<CorpusWriter> writer,
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
      "c:d:m:q:r"));
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  
  if (opts->arguments().size() == 0)
  {
    usage(opts->programName());
    return 1;
  }
  
  if (opts->option('c') && opts->option('d')) {
    std::cerr << opts->programName() <<
      ": the '-c' and 'd' options cannot be used simultaneously." <<
      std::endl;
    return 1;
  }
  
  if (!opts->option('c') && !opts->option('d')) {
    std::cerr << opts->programName() <<
    ": one of the '-c' or 'd' options should be used." <<
    std::endl;
    return 1;
  }
 
  boost::shared_ptr<CorpusReader> reader;
  try {
    if (opts->arguments().size() == 1)
      reader = boost::shared_ptr<CorpusReader>(
        openCorpus(opts->arguments().at(0), opts->option('r')));
    else
      reader = boost::shared_ptr<CorpusReader>(
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

    Either<std::string, alpinocorpus::Empty> valid =
      reader->isValidQuery(CorpusReader::XPATH, false, query);
    if (valid.isLeft()) {
      std::cerr << "Invalid (or unwanted) query: " << query << std::endl << std::endl;
      std::cerr << valid.left() << std::endl;
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
        boost::shared_ptr<CorpusWriter> wr(new DbCorpusWriter(treebankOut, true));
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
        std::string outIndex = treebankOut + ".index";
        std::string outDataDz = treebankOut + ".data.dz";

        // XXX - needs a more sophisticated check now, the output treebank
        // could also be in the search path of a recursive reader.
        for (std::vector<std::string>::const_iterator iter =
            opts->arguments().begin(); iter != opts->arguments().end();
            ++iter)
          if (bf::equivalent(outIndex, *iter) || bf::equivalent(outDataDz, *iter))
            throw std::runtime_error("Attempting to write to the source treebank.");
  
        boost::shared_ptr<CorpusWriter> wr(new CompactCorpusWriter(treebankOut));
        writeCorpus(reader, wr, query);

    } catch (std::runtime_error const &e) {
        std::cerr << opts->programName() <<
        ": error creating compact corpus: " << e.what() << std::endl;
        return 1;
    }    
  }
  
  return 0;
}
