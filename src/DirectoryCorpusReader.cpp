#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>

#include "DirectoryCorpusReaderPrivate.hh"

namespace alpinocorpus {

DirectoryCorpusReader::DirectoryCorpusReader(std::string const &directory,
    bool wantCache)
    : d_private(new DirectoryCorpusReaderPrivate(directory, wantCache))
{
}

DirectoryCorpusReader::~DirectoryCorpusReader()
{
}

CorpusReader::EntryIterator DirectoryCorpusReader::getBegin() const
{
    return d_private->getBegin();
}

CorpusReader::EntryIterator DirectoryCorpusReader::getEnd() const
{
    return d_private->getEnd();
}

size_t DirectoryCorpusReader::getSize() const
{
  return d_private->getSize();
}

std::string DirectoryCorpusReader::readEntry(std::string const &entry) const
{
  return d_private->readEntry(entry);
}

}   // namespace alpinocorpus
