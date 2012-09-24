#include <list>
#include <string>

#include <AlpinoCorpus/tr1wrap/memory.hh>

#include <boost/filesystem.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>

namespace bf = boost::filesystem;

namespace alpinocorpus {

class RecursiveCorpusReaderPrivate : public CorpusReader
{
public:
  RecursiveCorpusReaderPrivate(std::string const &directory,
      bool dactOnly);
  virtual ~RecursiveCorpusReaderPrivate();

  EntryIterator getEntries() const;
  std::string getName() const;
  size_t getSize() const;
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query) const;
  bool validQuery(QueryDialect d, bool variables, std::string const &query) const;

private:
  bf::path d_directory;
  std::tr1::shared_ptr<MultiCorpusReader> d_multiReader;
};


// Implementation of the public interface.
RecursiveCorpusReader::RecursiveCorpusReader(std::string const &directory,
    bool dactOnly) :
  d_private(new RecursiveCorpusReaderPrivate(directory, dactOnly))
{
}

RecursiveCorpusReader::~RecursiveCorpusReader()
{
  delete d_private;
}

CorpusReader::EntryIterator RecursiveCorpusReader::getEntries() const
{
  return d_private->getEntries();
}

std::string RecursiveCorpusReader::getName() const
{
  return d_private->getName();
}

size_t RecursiveCorpusReader::getSize() const
{
  return d_private->getSize();
}
    
bool RecursiveCorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
  return d_private->isValidQuery(d, variables, query);
}

std::string RecursiveCorpusReader::readEntry(std::string const &entry) const
{
  return d_private->readEntry(entry);
}
    
std::string RecursiveCorpusReader::readEntryMarkQueries(std::string const &entry, 
    std::list<MarkerQuery> const &queries) const
{
  return d_private->readEntryMarkQueries(entry, queries);
}

CorpusReader::EntryIterator RecursiveCorpusReader::runXPath(std::string const &query) const
{
  return d_private->query(XPATH, query);
}

// Implementation of the private interface

RecursiveCorpusReaderPrivate::RecursiveCorpusReaderPrivate(std::string const &directory,
    bool dactOnly) :
  d_multiReader(new MultiCorpusReader)
{
  if (directory[directory.size() - 1] == '/')
    d_directory = bf::path(directory).parent_path();
  else
    d_directory = bf::path(directory);
  
  if (!bf::exists(d_directory) ||
    !bf::is_directory(d_directory))
    throw OpenError(directory, "non-existent or not a directory");

  for (bf::recursive_directory_iterator iter(d_directory, bf::symlink_option::recurse);
       iter != bf::recursive_directory_iterator();
       ++iter)
  {
    if (iter->path().extension() != ".dact" &&
        (dactOnly || iter->path().extension() != ".index"))
      continue;

    bf::path namePath = iter->path();
    namePath.replace_extension("");
    std::string name = namePath.string();

    name.erase(0, d_directory.string().size() + 1);
   
    d_multiReader->push_back(name, iter->path().string(), false);
  }
}

RecursiveCorpusReaderPrivate::~RecursiveCorpusReaderPrivate()
{
}

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::getEntries() const
{
  return d_multiReader->entries();
}

std::string RecursiveCorpusReaderPrivate::getName() const
{
  return "<recursive>";
}

size_t RecursiveCorpusReaderPrivate::getSize() const
{
  return d_multiReader->size();
}

std::string RecursiveCorpusReaderPrivate::readEntry(std::string const &path) const
{
  return d_multiReader->read(path);
}

std::string RecursiveCorpusReaderPrivate::readEntryMarkQueries(
    std::string const &path, std::list<MarkerQuery> const &queries) const
{
  return d_multiReader->read(path, queries);
}

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::runXPath(
    std::string const &query) const
{
  return d_multiReader->query(CorpusReader::XPATH, query);
}

bool RecursiveCorpusReaderPrivate::validQuery(QueryDialect d, bool variables,
    std::string const &query) const
{
    if (!d_multiReader)
      return false;

    return d_multiReader->isValidQuery(d, variables, query);
}


}

