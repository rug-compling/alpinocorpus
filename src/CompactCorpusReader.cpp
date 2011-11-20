#include <string>

#include <AlpinoCorpus/CompactCorpusReader.hh>
#include "CompactCorpusReaderPrivate.hh"

namespace alpinocorpus {

CompactCorpusReader::CompactCorpusReader(std::string const &filename)
    : d_private(new CompactCorpusReaderPrivate(filename))
{
}

CompactCorpusReader::CompactCorpusReader(std::string const &dataPath,
    std::string const &indexPath)
    : d_private(new CompactCorpusReaderPrivate(dataPath, indexPath))
{
}

CompactCorpusReader::~CompactCorpusReader()
{
    delete d_private;
}

CorpusReader::EntryIterator CompactCorpusReader::getBegin() const
{
    return d_private->getBegin();
}

CorpusReader::EntryIterator CompactCorpusReader::getEnd() const
{
    return d_private->getEnd();
}

std::string CompactCorpusReader::getName() const
{
    return d_private->getName();
}

size_t CompactCorpusReader::getSize() const
{
    return d_private->getSize();
}

std::string CompactCorpusReader::readEntry(std::string const &filename) const
{
  return d_private->readEntry(filename);
}

}   // namespace alpinocorpus
