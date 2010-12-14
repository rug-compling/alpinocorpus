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
    /** Iterator over entry names */
    class EntryIterator
        : public std::iterator<std::input_iterator_tag, QString const>
    {
      public:
        EntryIterator(EntryIterator const &other) { copy(other); }
        EntryIterator &operator++() { next(); return *this; }
        EntryIterator operator++(int)
        {
            EntryIterator r(*this);
            operator++();
            return r;
        }
        bool operator==(EntryIterator const &other) { return equals(other); }
        bool operator!=(EntryIterator const &other) { return !equals(other); }
        value_type &operator*() { return current(); }

      protected:
        EntryIterator() { }

      private:
        virtual void copy(EntryIterator const &);
        virtual value_type &current();
        virtual bool equals(EntryIterator const &);
        virtual void next();
    };

    virtual ~CorpusReader() {}

    /** Return canonical name of corpus */
    virtual QString name() const = 0;

    /** Retrieve the names of all treebank entries. */
    virtual QVector<QString> entries() const = 0;

    /** Iterator to begin of entry names */
    virtual EntryIterator begin() const = 0;

    /** Iterator to end of entry names */
    virtual EntryIterator end() const = 0;

    /** Return content of a single treebank entry. */
    virtual QString read(QString const &entry) = 0;

    /** Factory method: open a corpus, determining its type automatically. */
    static INDEXED_CORPUS_EXPORT CorpusReader *newCorpusReader(QString const &corpusPath);
};

}

#endif  // ALPINO_CORPUSREADER_HH
