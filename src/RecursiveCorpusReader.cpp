#include <filesystem>
#include <list>
#include <memory>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>

namespace fs = std::filesystem;

namespace alpinocorpus {

class RecursiveCorpusReaderPrivate : public CorpusReader
{
public:
  RecursiveCorpusReaderPrivate(std::string const &directory,
      bool dactOnly);
  virtual ~RecursiveCorpusReaderPrivate();

  EntryIterator getEntries(SortOrder sortOrder) const;
  std::string getName() const;
  size_t getSize() const;
  std::string readEntry(std::string const &) const;
  std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
  EntryIterator runXPath(std::string const &query) const;
  EntryIterator runXQuery(std::string const &, SortOrder sortOrder) const;
  Either<std::string, Empty> validQuery(QueryDialect d, bool variables, std::string const &query) const;

private:
  fs::path d_directory;
  std::shared_ptr<MultiCorpusReader> d_multiReader;
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

CorpusReader::EntryIterator RecursiveCorpusReader::getEntries(SortOrder sortOrder) const
{
  return d_private->getEntries(sortOrder);
}

std::string RecursiveCorpusReader::getName() const
{
  return d_private->getName();
}

size_t RecursiveCorpusReader::getSize() const
{
  return d_private->getSize();
}

Either<std::string, Empty> RecursiveCorpusReader::validQuery(QueryDialect d, bool variables, std::string const &query) const
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

CorpusReader::EntryIterator RecursiveCorpusReader::runXQuery(std::string const &query, SortOrder sortOrder) const
{
  return d_private->query(XQUERY, query, sortOrder);
}

// Implementation of the private interface

RecursiveCorpusReaderPrivate::RecursiveCorpusReaderPrivate(std::string const &directory,
    bool dactOnly) :
  d_multiReader(new MultiCorpusReader)
{
  if (directory[directory.size() - 1] == '/')
    d_directory = fs::path(directory).parent_path();
  else
    d_directory = fs::path(directory);

  if (!fs::exists(d_directory) ||
    !fs::is_directory(d_directory))
    throw OpenError(directory, "non-existent or not a directory");

  for (fs::recursive_directory_iterator iter(d_directory,
       fs::directory_options::follow_directory_symlink);
       iter != fs::recursive_directory_iterator();
       ++iter)
  {
    if (iter->path().extension() != ".dact" &&
        (dactOnly || iter->path().extension() != ".index"))
      continue;

    fs::path namePath = iter->path();
    namePath.replace_extension("");
    std::string name = namePath.string();

    name.erase(0, d_directory.string().size() + 1);

    d_multiReader->push_back(name, iter->path().string(), false);
  }
}

RecursiveCorpusReaderPrivate::~RecursiveCorpusReaderPrivate()
{
}

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::getEntries(SortOrder sortOrder) const
{
  return d_multiReader->entries(sortOrder);
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

CorpusReader::EntryIterator RecursiveCorpusReaderPrivate::runXQuery(
    std::string const &query, SortOrder sortOrder) const
{
  return d_multiReader->query(CorpusReader::XQUERY, query, sortOrder);
}

Either<std::string, Empty> RecursiveCorpusReaderPrivate::validQuery(QueryDialect d, bool variables,
    std::string const &query) const
{
    if (!d_multiReader)
      return Either<std::string, Empty>::left("No reader available.");

    return d_multiReader->isValidQuery(d, variables, query);
}


}
