#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include <tr1/memory>
#include <tr1/unordered_set>

#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>

#include <iostream>
#include <stdexcept>

#include "ProgramOptions.hh"

using alpinocorpus::CorpusReader;
using alpinocorpus::DbCorpusWriter;
using alpinocorpus::MultiCorpusReader;

namespace bf = boost::filesystem;

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

CorpusReader *openCorpora(std::vector<std::string> const &paths,
    bool recursive)
{
  MultiCorpusReader *readers = new MultiCorpusReader;

  for (std::vector<std::string>::const_iterator iter = paths.begin();
      iter != paths.end(); ++iter)
  {
    CorpusReader *reader = openCorpus(*iter, recursive);
    if (reader == 0) {
      delete readers;
      return 0;
    }

    // If we are dealing with a directory, and the path ends with a trailing
    // slash, we remove the slash.
    bf::path p = bf::path(*iter);
    if (bf::is_directory(p) && iter->rfind('/') == iter->size() - 1)
      p = bf::path(iter->substr(0, iter->size() - 1));

    // Kill the extension, if there is any.
    p.replace_extension("");

    // Use the last path component as the corpus name.
    std::string name = p.filename().generic_string();


    // Strip extensions

    readers->push_back(name, reader);
  }

  return readers;
}

void listCorpus(std::tr1::shared_ptr<CorpusReader> reader,
  std::string const &query)
{
  CorpusReader::EntryIterator i, end(reader->end());
  
  if (query.empty())
    i = reader->begin();
  else
    i = reader->query(CorpusReader::XPATH, query);

  std::copy(i, end, std::ostream_iterator<std::string>(std::cout, "\n"));  
}

void readEntry(std::tr1::shared_ptr<CorpusReader> reader, std::string const &entry)
{
  std::cout << reader->read(entry);
}

void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] treebank" <<
      std::endl << std::endl <<
      "  -c filename\tCreate a Dact dbxml archive" << std::endl <<
      "  -g entry\tPrint a treebank entry to stdout" << std::endl <<
      "  -l\t\tList the entries of a treebank" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl <<
      "  -r\t\tProcess a directory of corpora recursively" << std::endl << std::endl;
}

void writeDactCorpus(std::tr1::shared_ptr<CorpusReader> reader,
  std::string const &treebankOut,
  std::string const &query)
{
  DbCorpusWriter wr(treebankOut, true);
  CorpusReader::EntryIterator i, end(reader->end());
  if (query.empty())
    i = reader->begin();
  else
    i = reader->query(CorpusReader::XPATH, query);
  
  std::tr1::unordered_set<std::string> seen;
  for (; i != end; ++i)
    if (seen.find(*i) == seen.end()) {
        wr.write(*i, reader->read(*i));
      seen.insert(*i);
    }
}

int main(int argc, char *argv[])
{
  boost::scoped_ptr<ProgramOptions> opts;
  try {
    opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
      "c:g:lq:r"));
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
  char const commands[] = "cgl";
  for (size_t i = 0; i < 3; ++i)
    if (opts->option(commands[i]))
      ++cmdCount;
  
  if (cmdCount > 1) {
    std::cerr << opts->programName() <<
      ": the '-c', '-g', and '-l' options cannot be used simultaneously." <<
      std::endl;
    return 1;
  }
  
  if (cmdCount == 0) {
    std::cerr << opts->programName() <<
    ": one of the '-c', '-g' or '-l' option should be used." <<
    std::endl;
    return 1;
  }
 
  std::tr1::shared_ptr<CorpusReader> reader;
  if (opts->arguments().size() == 1)
    reader = std::tr1::shared_ptr<CorpusReader>(
      openCorpus(opts->arguments().at(0), opts->option('r')));
  else
    reader = std::tr1::shared_ptr<CorpusReader>(
      openCorpora(opts->arguments(), opts->option('r')));

  if (reader.get() == 0)
    return 1;
  
  std::string query;
  if (opts->option('q')) {
    query = opts->optionValue('q');  

    if (!reader->isValidQuery(CorpusReader::XPATH, false, query)) {
      std::cerr << "Invalid (or unwanted) query: " << query << std::endl;
      return 1;
    }
  }
  
  if (opts->option('c')) {
    try {
        std::string treebankOut = opts->optionValue('c').c_str();

        // XXX - needs a more sophisticated check now, the output treebank
        // could also be in the search path of a recursive reader.
        for (std::vector<std::string>::const_iterator iter =
            opts->arguments().begin(); iter != opts->arguments().end();
            ++iter)
          if (bf::equivalent(treebankOut, *iter))
            throw std::runtime_error("Attempting to write to the source treebank.");
  
      writeDactCorpus(reader, treebankOut, query);
    } catch (std::runtime_error const &e) {
        std::cerr << opts->programName() <<
        ": error creating Dact treebank: " << e.what() << std::endl;
        return 1;
    }
  }

  if (opts->option('g')) {
    try {
      readEntry(reader, opts->optionValue('g'));
    } catch (std::runtime_error const &e) {
        std::cerr << opts->programName() <<
        ": error reading entry: " << e.what() << std::endl;
        return 1;
    }    
  }
  
  if (opts->option('l')) {
    try {
        listCorpus(reader, query);
    } catch (std::runtime_error const &e) {
        std::cerr << opts->programName() <<
        ": error listing treebank: " << e.what() << std::endl;
        return 1;
    }    
  }
  
  return 0;
}
