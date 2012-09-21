#include <list>
#include <string>

#include <dbxml/DbXml.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/IterImpl.hh>

namespace alpinocorpus {

class DbCorpusReaderPrivate : public CorpusReader
{
    // XXX mutable is hideous, but saves a lot of const_casts: the read
    // methods are nominally const (don't change future behavior and are
    // thread-safe), but DB XML doesn't expose const reading methods.
    DbXml::XmlManager   mutable mgr;
    DbXml::XmlContainer mutable container;
    std::string collection;
    
    class DbIter : public IterImpl
    {
    public:
        DbIter(DbXml::XmlContainer &);
        DbIter(DbXml::XmlManager &);

        virtual IterImpl *copy() const;        
        bool equals(IterImpl const &) const;
        bool hasNext();
        Entry next(CorpusReader const &);
        
    protected:
        mutable DbXml::XmlResults r;
        
        DbIter(DbXml::XmlResults const &);
    };
    
    class QueryIter : public DbIter
    {
    public:
        QueryIter(DbXml::XmlResults const &, DbXml::XmlQueryContext const &);
        IterImpl *copy() const;
        void interrupt();
    
    protected:
        DbXml::XmlQueryContext context;
    };
    
public:
    DbCorpusReaderPrivate(std::string const &);
    virtual ~DbCorpusReaderPrivate();
    EntryIterator getEntries() const;
    std::string getName() const;
    size_t getSize() const
    {
        return const_cast<DbXml::XmlContainer &>(container).getNumDocuments();
    }
    bool validQuery(QueryDialect d, bool variables, std::string const &query) const;
    std::string readEntry(std::string const &) const;
    EntryIterator runXPath(std::string const &) const;
    EntryIterator runXQuery(std::string const &) const;
    
private:
    void setNameAndCollection(std::string const &);
    
};

}

