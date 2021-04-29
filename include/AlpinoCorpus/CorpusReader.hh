#ifndef ALPINO_CORPUSREADER_HH
#define ALPINO_CORPUSREADER_HH

#include <cstddef>
#include <list>
#include <memory>
#include <queue>
#include <string>

#include <AlpinoCorpus/CorpusInfo.hh>
#include <AlpinoCorpus/DLLDefines.hh>
#include <AlpinoCorpus/IterImpl.hh>
#include <AlpinoCorpus/LexItem.hh>
#include <AlpinoCorpus/util/Either.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

namespace alpinocorpus {

class XSLTransformer;

/**
 * The sort order for iterators.
 *
 * Note that this is currently just a hint to the iterator implementation.
 * Most iterators currently only support the order that is natural to the
 * underlying corpus.
 */
enum SortOrder {
    /**
     * Sort using the natural order, typically the sequence in which the
     * entries are stored in a treebank.
     */
    NaturalOrder,

    /**
     * Sort using numeric order.
     */
    NumericalOrder
};

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
    {
      public:
        EntryIterator();
        EntryIterator(IterImpl *p);
        EntryIterator(EntryIterator const &other);
        virtual ~EntryIterator();
        EntryIterator &operator=(EntryIterator const &other);
        bool hasNext();
        bool hasProgress() const;
        Entry next(CorpusReader const &reader);
        double progress() const;

        /**
         * Interrupt an iterator that is blocking.
         */
        void interrupt();

      private:
        void copy(EntryIterator const &other);

        std::shared_ptr<IterImpl> d_impl;
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

    /** Iterator over entry names. */
    EntryIterator entries(SortOrder order = NaturalOrder) const;

    /**
     * Iterator over entry names, contents are transformed with
     * the given stylesheet.
     */
    EntryIterator entriesWithStylesheet(std::shared_ptr<XSLTransformer> const stylesheet,
      std::list<MarkerQuery> const &markerQueries = std::list<MarkerQuery>(),
      SortOrder sortOrder = NaturalOrder) const;

    enum QueryDialect { XPATH, XQUERY };

    /** Is a query valid? */
    Either<std::string, Empty> isValidQuery(QueryDialect d, bool variables, std::string const &q) const;
    
    /** Execute query. The end of the range is given by end(). */
    EntryIterator query(QueryDialect d, std::string const &q,
        SortOrder sortOrder = NaturalOrder) const;

    /**
     * Execute a query, applying the given stylesheet to each entry. The
     * end of the range is given by end().
     */ 
    EntryIterator queryWithStylesheet(QueryDialect d, std::string const &q,
        std::shared_ptr<XSLTransformer> const stylesheet,
        std::list<MarkerQuery> const &markerQueries,
        SortOrder sortOrder = NaturalOrder) const;
    
    /**
     * Return content of a single treebank entry. Mark elements if a marker
     * queries were provided.
     */
    std::string read(std::string const &entry,
      std::list<MarkerQuery> const &queries = std::list<MarkerQuery>()) const;

    /**
     * Retrieve a sentence. The sentence is returned as a list of lexical
     * items, where each item contains the (query) match depth. The
     * attribute that should be returned can also be specified (such as
     * 'word' for words).
     *
     * Note: in the future we might change LexItem to contain all
     * attribute/value pairs.
     */
    std::vector<LexItem> sentence(std::string const &entry,
      std::string const &query, std::string const &attribute,
      std::string const &defaultValue,
      CorpusInfo const &corpusInfo) const;

    /** The treebank  type. For now, this is defined to be the name of the root
     *  element, e.g. 'alpino_ds' for Alpino treebanks. */
    std::string type() const;

    /** The number of entries in the corpus. */
    size_t size() const;

  private:
    virtual EntryIterator getEntries(SortOrder sortOrder) const = 0;
    virtual std::string getName() const = 0;
    virtual std::vector<LexItem> getSentence(std::string const &entry,
        std::string const &query, std::string const &attribute,
        std::string const &defaultValue, CorpusInfo const &corpusInfo) const;
    virtual size_t getSize() const = 0;
    virtual std::string readEntry(std::string const &entry) const = 0;
    virtual std::string readEntryMarkQueries(std::string const &entry,
        std::list<MarkerQuery> const &queries) const;
    virtual EntryIterator runXPath(std::string const &, SortOrder sortOrder) const;
    virtual EntryIterator runXQuery(std::string const &, SortOrder sortOrder) const;
    virtual EntryIterator runQueryWithStylesheet(QueryDialect d,
      std::string const &q, std::shared_ptr<XSLTransformer> const stylesheet,
      std::list<MarkerQuery> const &markerQueries, SortOrder sortOrder) const;
    virtual Either<std::string, Empty> validQuery(QueryDialect d, bool variables, std::string const &q) const;

    // Initialized lazily in type();
    std::shared_ptr<std::string> d_type;
};

}

#endif  // ALPINO_CORPUSREADER_HH
