#ifndef ALPINO_CORPUSREADER_HH
#define ALPINO_CORPUSREADER_HH

#include <cstddef>
#include <list>
#include <queue>
#include <string>

#include <AlpinoCorpus/DLLDefines.hh>
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
  protected:
    // Iterator body. We need handle-body/proxy/pimpl for polymorphic copy.
    struct IterImpl {
        virtual ~IterImpl() {}
        virtual IterImpl *copy() const = 0;
        virtual std::string current() const = 0;
        virtual bool equals(IterImpl const &) const = 0;
        virtual void next() = 0;

        // Query iterators must override this
        virtual std::string contents(CorpusReader const &rdr) const;
        virtual void interrupt();
    };

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

    /**
     * Reader types
     */
    enum ReaderType {
        DIRECTORY_CORPUS_READER = 0,
        COMPACT_CORPUS_READER,
        DBXML_CORPUS_READER
    };

    /**
     * Reader information
     */
    struct ReaderInfo {
      ReaderInfo(ReaderType newType, std::string newDescription,
          std::list<std::string> newExtensions) :
        readerType(newType),
        description(newDescription),
        extensions(newExtensions) {}

      ReaderType readerType;
      std::string description;
      std::list<std::string> extensions;
    };
    
    struct MarkerQuery {
        MarkerQuery(std::string const &newQuery, std::string const &newAttr,
            std::string const &newValue) :
            query(newQuery), attr(newAttr), value(newValue) {}
        std::string query;
        std::string attr;
        std::string value;
    };

    virtual ~CorpusReader() {}

    /** Return canonical name of corpus */
    std::string name() const;

    /** Iterator to begin of entry names */
    EntryIterator begin() const;

    /** Iterator to end of entry names */
    EntryIterator end() const;

    enum QueryDialect { XPATH, XQUERY };

    /** Is a query valid? */
    bool isValidQuery(QueryDialect d, bool variables, std::string const &q) const;
    
    /** Execute query. The end of the range is given by end(). */
    EntryIterator query(QueryDialect d, std::string const &q) const;

    /** Return content of a single treebank entry. */
    std::string read(std::string const &entry) const;
    
    /**
     * Return content of a single treebank entry, marking matching elements using the
     * given attribute and value.
     */
    std::string readMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;

    /** The number of entries in the corpus. */
    size_t size() const;
    
    /**
     * Factory method: open a corpus, determining its type automatically.
     * The caller is responsible for deleting the object returned.
     */
    static CorpusReader *open(std::string const &corpusPath);

    /**
     * Factory method: open a directory with multiple corpora recursively.
     * The caller is responsible for deleting the object returned.
     */
    static CorpusReader *openRecursive(std::string const &path);

    /**
     * Check whether a particular reader type is available.
     */
     static bool readerAvailable(ReaderType readerType);

    /**
     * Retrieve a list of available corpus readers.
     */
     static std::list<ReaderInfo> readersAvailable();

  protected:
    class FilterIter : public IterImpl {
      public:
        FilterIter(CorpusReader const &, EntryIterator, EntryIterator, std::string const &);
        IterImpl *copy() const;
        std::string current() const;
        bool equals(IterImpl const &) const;
        void next();
        std::string contents(CorpusReader const &) const;

      protected:
        void interrupt();
      
      private:
        void parseFile(std::string const &);
        
        CorpusReader const &d_corpus;
        EntryIterator d_itr;
        EntryIterator d_end;
        std::string d_file;
        std::string d_query;
        std::queue<std::string> d_buffer;
        mutable bool d_initialState;
        bool d_interrupted;
    };

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
