#ifndef ALPINO_CORPUSREADER_HH
#define ALPINO_CORPUSREADER_HH

#include <QString>
#include <QVector>

#include <AlpinoCorpus/DLLDefines.hh>

namespace alpinocorpus {

/**
 * Abstract base class for corpus readers.
 *
 * A corpus is conceptually a mapping of names to XML documents.
 * Both are represented as QStrings.
 */
class CorpusReader
{
public:
    virtual ~CorpusReader() {}

    /** Return canonical name of corpus */
    virtual QString name() const = 0;

    /** Open corpus for reading. Returns true on success, false on failure. */
    virtual bool open() = 0;

    /** Retrieve the names of all treebank entries. */
    virtual QVector<QString> entries() const = 0;

    /** Return content of a single treebank entry. */
    virtual QString read(QString const &entry) = 0;

    /** Factory method: open a corpus, determining its type automatically. */
    static INDEXED_CORPUS_EXPORT CorpusReader *newCorpusReader(QString const &corpusPath);
};

}

#endif  // ALPINO_CORPUSREADER_HH
