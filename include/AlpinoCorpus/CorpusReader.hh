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
    QString name_;

  protected:
    // Iterator body. We need handle-body/proxy/pimpl for polymorphic copy.
    struct IterImpl {
        virtual QString current() const = 0;
        virtual bool equals(IterImpl const *) const = 0;
        virtual void next() = 0;
    };

    void setName(QString const &n) { name_ = n; }

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
        value_type operator*() const { return impl->current(); }
        //value_type const *operator->() const { return impl->current(); }
    };

    virtual ~CorpusReader() {}

    /** Return canonical name of corpus */
    QString const &name() const { return name_; }

    /** Iterator to begin of entry names */
    EntryIterator begin() const { return getBegin(); }

    /** Iterator to end of entry names */
    EntryIterator end() const { return getEnd(); }

    /** Execute XPath query. The end of the range is given by end(). */
    EntryIterator query(QString const &q) const { return runQuery(q); }

    /** Return content of a single treebank entry. */
    QString read(QString const &entry) const { return readEntry(entry); }

    /** The number of entries in the corpus. */
    size_t size() const { return getSize(); }

    /**
     * Factory method: open a corpus, determining its type automatically.
     * The caller is responsible for deleting the object returned.
     */
    static INDEXED_CORPUS_EXPORT CorpusReader *open(QString const &corpusPath);

  private:
    virtual EntryIterator getBegin() const = 0;
    virtual EntryIterator getEnd() const = 0;
    virtual size_t getSize() const = 0;
    virtual QString readEntry(QString const &entry) const = 0;
    virtual EntryIterator runQuery(QString const &) const;
};

}

#endif  // ALPINO_CORPUSREADER_HH
