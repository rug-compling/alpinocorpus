#ifndef ALPINO_CORPUSREADER_HH
#define ALPINO_CORPUSREADER_HH

#include <QSharedPointer>
#include <QString>
#include <QVector>

// for FilterIter
#include <QByteArray>
#include <QQueue>

#include <AlpinoCorpus/DLLDefines.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

namespace alpinocorpus {

/**
 * Abstract base class for corpus readers.
 *
 * A corpus is conceptually a mapping of names to XML documents.
 * Both are represented as QStrings.
 */
class ALPINO_CORPUS_EXPORT CorpusReader : private util::NonCopyable
{
  protected:
    // Iterator body. We need handle-body/proxy/pimpl for polymorphic copy.
    struct IterImpl {
        virtual QString current() const = 0;
        virtual bool equals(IterImpl const &) const = 0;
        virtual void next() = 0;

        // Query iterators must override this
        virtual QString contents(CorpusReader const &rdr) const;
    };

    void setName(QString const &n);
    
  public:
    /** Forward iterator over entry names */
    class ALPINO_CORPUS_EXPORT EntryIterator
        : public std::iterator<std::input_iterator_tag, QString>
    {
        QSharedPointer<IterImpl> impl;

      public:
        EntryIterator() {}
        EntryIterator(IterImpl *p) : impl(p) { }
        EntryIterator(EntryIterator const &other) : impl(other.impl) { }
        EntryIterator &operator++();
        EntryIterator operator++(int);
        bool operator==(EntryIterator const &other) const;
        bool operator!=(EntryIterator const &other) const;
        value_type operator*() const { return impl->current(); }
        //value_type const *operator->() const { return impl->current(); }

        /**
         * Get contents of entry pointed to by iterator.
         * This will be a null string for an ordinary iterator,
         * and the matching part for a query iterator.
         */
        QString contents(CorpusReader const &rdr) const;
    };

    virtual ~CorpusReader() {}

    /** Return canonical name of corpus */
    QString const &name() const;

    /** Iterator to begin of entry names */
    EntryIterator begin() const;

    /** Iterator to end of entry names */
    EntryIterator end() const;

    enum QueryDialect { XPATH, XQUERY };

    /** Is a query valid? */
    bool isValidQuery(QueryDialect d, bool variables, QString const &q) const;
    
    /** Execute query. The end of the range is given by end(). */
    EntryIterator query(QueryDialect d, QString const &q) const;

    /** Return content of a single treebank entry. */
    QString read(QString const &entry) const;

    /** The number of entries in the corpus. */
    size_t size() const;
    
    /**
     * Factory method: open a corpus, determining its type automatically.
     * The caller is responsible for deleting the object returned.
     */
    static CorpusReader *open(QString const &corpusPath);

  protected:
    class FilterIter : public IterImpl {
      public:
        FilterIter(CorpusReader const &, EntryIterator, EntryIterator, QString const &);
        QString current() const;
        bool equals(IterImpl const &) const;
        void next();
        QString contents(CorpusReader const &) const;
      
      private:
        void parseFile(QString const &);
        
        CorpusReader const &d_corpus;
        EntryIterator d_itr;
        EntryIterator d_end;
        QString d_file;
        QByteArray d_query;
        QQueue<QString> d_buffer;
    };

  private:
    virtual EntryIterator getBegin() const = 0;
    virtual EntryIterator getEnd() const = 0;
    virtual size_t getSize() const = 0;
    virtual QString readEntry(QString const &entry) const = 0;
    virtual EntryIterator runXPath(QString const &) const;
    virtual EntryIterator runXQuery(QString const &) const;
    virtual bool validQuery(QueryDialect d, bool variables, QString const &q) const;
    
    QString d_name;
};

inline CorpusReader::EntryIterator CorpusReader::begin() const
{
    return getBegin();
}
    
inline CorpusReader::EntryIterator CorpusReader::end() const
{
    return getEnd();
}

inline bool CorpusReader::isValidQuery(QueryDialect d, bool variables, QString const &q) const
{
    return validQuery(d, variables, q);
}

inline QString const &CorpusReader::name() const
{
    return d_name;
}

inline QString CorpusReader::read(QString const &entry) const
{
    return readEntry(entry);
}

inline void CorpusReader::setName(QString const &n)
{
    d_name = n;
}

inline size_t CorpusReader::size() const
{
    return getSize();
}

    
inline QString CorpusReader::IterImpl::contents(CorpusReader const &rdr) const
{
    //return rdr.read(current());
    return QString();
}

    
inline bool CorpusReader::EntryIterator::operator!=(EntryIterator const &other) const
{
    return !operator==(other);
}

inline QString CorpusReader::EntryIterator::contents(CorpusReader const &rdr) const
{
    return impl->contents(rdr);
}



}

#endif  // ALPINO_CORPUSREADER_HH
