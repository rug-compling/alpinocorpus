#ifndef ALPINO_CORPUSREADER_HH
#define ALPINO_CORPUSREADER_HH

#include <QSharedPointer>
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
  protected:
    // Iterator body. We need handle-body/proxy/pimpl for polymorphic copy.
    struct IterImpl {
        virtual QString const &current() const = 0;
        virtual bool equals(IterImpl const *) const = 0;
        virtual void next() = 0;
        virtual size_t size() const { throw SizeNotAvailable(); }
    };

  public:
    /** Forward iterator over entry names */
    class EntryIterator
        : public std::iterator<std::input_iterator_tag, QString>
    {
        QSharedPointer<IterImpl> impl;

      public:
        EntryIterator(IterImpl *p) : impl(p) { }
        EntryIterator(EntryIterator const &other) : impl(other.impl) { }
        EntryIterator &operator++() { impl->next(); return *this; }
        EntryIterator operator++(int)
        {
            EntryIterator r(*this);
            operator++();
            return r;
        }
        bool operator==(EntryIterator const &other)
        { return impl->equals(other.impl.data()); }
        bool operator!=(EntryIterator const &other)
        { return !operator==(other); }
        value_type const &operator*() const { return impl->current(); }
        //value_type const *operator->() const { return impl->current(); }

        /** Size of corpus (subset) pointed to */
        size_t size() const throw(SizeNotAvailable)
        { return impl.size(); }
    };

    virtual ~CorpusReader() {}

    /** Return canonical name of corpus */
    virtual QString name() const = 0;

    /**
     * Retrieve the names of all treebank entries.
     * @deprecated Use the iterator interface instead.
     */
    virtual QVector<QString> entries() const = 0;

    /** Iterator to begin of entry names */
    virtual EntryIterator begin() const = 0;

    /** Iterator to end of entry names */
    virtual EntryIterator end() const = 0;

    /** Return content of a single treebank entry. */
    virtual QString read(QString const &entry) = 0;

    /** The number of entries in the corpus. */
    virtual size_t size() const = 0;

    /**
     * Factory method: open a corpus, determining its type automatically.
     * The caller is responsible for deleting the object returned.
     */
    static INDEXED_CORPUS_EXPORT CorpusReader *open(QString const &corpusPath);
};

}

#endif  // ALPINO_CORPUSREADER_HH
