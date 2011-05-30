#ifndef ALPINO_INDEXED_CORPUSREADER_HH
#define ALPINO_INDEXED_CORPUSREADER_HH

#include <QSharedPointer>
#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus
{

class IndexedCorpusReaderPrivate;

class IndexedCorpusReader : public CorpusReader
{
public:
    /**
     * Construct from a single file (data or index); the other file is sought
     * for in the same directory.
     */
    IndexedCorpusReader(QString const &path);
    /** Construct from data and index file. */
    IndexedCorpusReader(QString const &dataFilename, QString const &indexFilename);
    virtual ~IndexedCorpusReader();

private:
    virtual EntryIterator getBegin() const;
    virtual EntryIterator getEnd() const;
    virtual QString readEntry(QString const &filename) const;
    virtual size_t getSize() const;

    QSharedPointer<IndexedCorpusReaderPrivate> d_private;
};

}

#endif  // ALPINO_INDEXED_CORPUSWRITER_HH
