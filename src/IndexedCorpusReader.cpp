#include <QString>

#include <AlpinoCorpus/IndexedCorpusReader.hh>
#include "IndexedCorpusReaderPrivate.hh"

namespace alpinocorpus {

IndexedCorpusReader::IndexedCorpusReader(QString const &filename)
    : d_private(new IndexedCorpusReaderPrivate(filename))
{
}

IndexedCorpusReader::IndexedCorpusReader(QString const &dataPath,
    QString const &indexPath)
    : d_private(new IndexedCorpusReaderPrivate(dataPath, indexPath))
{
}

IndexedCorpusReader::~IndexedCorpusReader()
{
}

CorpusReader::EntryIterator IndexedCorpusReader::getBegin() const
{
    return d_private->getBegin();
}

CorpusReader::EntryIterator IndexedCorpusReader::getEnd() const
{
    return d_private->getEnd();
}

size_t IndexedCorpusReader::getSize() const
{
    return d_private->getSize();
}

QString IndexedCorpusReader::readEntry(QString const &filename) const
{
  return d_private->readEntry(filename);
}

}   // namespace alpinocorpus
