#ifndef ALPINO_CORPUSREADER_HH
#define ALPINO_CORPUSREADER_HH

#include <cstddef>
#include <list>
#include <queue>
#include <string>

#include <AlpinoCorpus/DLLDefines.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

namespace alpinocorpus {

/**
 * Abstract base class for corpus readers.
 *
 * A corpus is conceptually a mapping of names to XML documents.
 * Both are represented as strings.
 */
class ALPINO_CORPUS_EXPORT CorpusReader : private util::NonCopyable
{
  public:
    /** Forward iterator over entry names */
    class ALPINO_CORPUS_EXPORT EntryIterator
    : public std::iterator<std::input_iterator_tag, std::string, ptrdiff_t, std::string *, std::string>
    {
      public:
        EntryIterator();
        EntryIterator(IterImpl *p);
        EntryIterator(EntryIterator const &other);
        virtual ~EntryIterator();
        EntryIterator &operator=(EntryIterator const &other);
        EntryIterator &operator++();
        EntryIterator operator++(int);
        bool operator==(EntryIterator const &other) const;
        bool operator!=(EntryIterator const &other) const;
        value_type operator*() const;
        //value_type const *operator->() const { return impl->current(); }

        /**
         * Get contents of entry pointed to by iterator.
         * This will be a null string for an ordinary iterator,
         * and the matching part for a query iterator.
         */
        std::string contents(CorpusReader const &rdr) const;

        /**
         * Interrupt an iterator that is blocking.
         */
        void interrupt();

      private:
        void copy(EntryIterator const &other);

        IterImpl *d_impl;
    };
    
    struct MarkerQuery {
        MarkerQuery(std::string const &newQuery, std::string const &newAttr,
            std::string const &newValue) :
            query(newQuery), attr(newAttr), value(newValue) {}
        std::string query;
        std::string attr;
        std::string value;

        bool operator==(MarkerQuery const &other) const;
    };

    virtual ~CorpusReader() {}

    /** Return canonical name of corpus */
    std::string name() const;

    /** Iterator to begin of entry names */
    EntryIterator begin() const;

    /**
     * Iterator to begin of entry names, contents are transformed with
     * the given stylesheet.
     */
    EntryIterator beginWithStylesheet(std::string const &stylesheet,
      std::list<MarkerQuery> const &markerQueries = std::list<MarkerQuery>()) const;

    /** Iterator to end of entry names */
    EntryIterator end() const;

    enum QueryDialect { XPATH, XQUERY };

    /** Is a query valid? */
    bool isValidQuery(QueryDialect d, bool variables, std::string const &q) const;
    
    /** Execute query. The end of the range is given by end(). */
    EntryIterator query(QueryDialect d, std::string const &q) const;

    /**
     * Execute a query, applying the given stylesheet to each entry. The
     * end of the range is given by end().
     */ 
    EntryIterator queryWithStylesheet(QueryDialect d, std::string const &q,
      std::string const &stylesheet,
      std::list<MarkerQuery> const &markerQueries) const;
    
    /**
     * Return content of a single treebank entry. Mark elements if a marker
     * queries were provided.
     */
    std::string read(std::string const &entry,
      std::list<MarkerQuery> const &queries = std::list<MarkerQuery>()) const;

    /** The number of entries in the corpus. */
    size_t size() const;

  private:
    virtual EntryIterator getBegin() const = 0;
    virtual EntryIterator getEnd() const = 0;
    virtual std::string getName() const = 0;
    virtual size_t getSize() const = 0;
    virtual std::string readEntry(std::string const &entry) const = 0;
    virtual std::string readEntryMarkQueries(std::string const &entry,
        std::list<MarkerQuery> const &queries) const;
    virtual EntryIterator runXPath(std::string const &) const;
    virtual EntryIterator runXQuery(std::string const &) const;
    virtual bool validQuery(QueryDialect d, bool variables, std::string const &q) const;
};

}

#endif  // ALPINO_CORPUSREADER_HH
