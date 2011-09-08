/*
 * Oracle Berkeley DB XML-based treebank reader.
 * Written by Lars Buitinck.
 *
 * The basic format is explained in the header. An additional notes:
 *  - We use DBML_WELL_FORMED_ONLY to prevent having to read the DTD.
 *    This only works because the DTD does not define entities.
 */

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <util/markqueries.hh>
#include <util/url.hh>

#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <iostream>

#include <dbxml/DbXml.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

#include <xqilla/xqilla-dom3.hpp>

namespace db = DbXml;

namespace xerces = XERCES_CPP_NAMESPACE;

namespace alpinocorpus {

class DbCorpusReaderPrivate : public CorpusReader
{
    // XXX mutable is hideous, but saves a lot of const_casts: the read
    // methods are nominally const (don't change future behavior and are
    // thread-safe), but DB XML doesn't expose const reading methods.
    DbXml::XmlManager   mutable mgr;
    DbXml::XmlContainer mutable container;
    std::string collection;
    
    class DbIter : public CorpusReader::IterImpl
    {
    public:
        DbIter(DbXml::XmlContainer &);
        DbIter(DbXml::XmlManager &);
        
        std::string current() const;
        bool equals(IterImpl const &) const;
        void next();
        
    protected:
        mutable DbXml::XmlResults r;
        
        DbIter(DbXml::XmlResults const &);
    };
    
    struct QueryIter : public DbIter
    {
        QueryIter(DbXml::XmlResults const &);
        std::string contents(CorpusReader const &) const;
    };
    
public:
    DbCorpusReaderPrivate(std::string const &);
    virtual ~DbCorpusReaderPrivate();
    EntryIterator getBegin() const;
    EntryIterator getEnd() const;
    std::string getName() const;
    size_t getSize() const
    {
        return const_cast<DbXml::XmlContainer &>(container).getNumDocuments();
    }
    std::string readEntry(std::string const &) const;
    std::string readEntryMarkQueries(std::string const &entry, std::list<MarkerQuery> const &queries) const;
    EntryIterator runXPath(std::string const &) const;
    EntryIterator runXQuery(std::string const &) const;
    
private:
    void setNameAndCollection(std::string const &);
    
};
    
DbCorpusReader::DbCorpusReader(std::string const &name) :
    d_private(new DbCorpusReaderPrivate(name))
{
}

DbCorpusReader::~DbCorpusReader()
{
}

/* begin() */
DbCorpusReaderPrivate::DbIter::DbIter(db::XmlContainer &container)
{
    try {
        r = container.getAllDocuments( db::DBXML_LAZY_DOCS
                                     | db::DBXML_WELL_FORMED_ONLY
                                     );
    } catch (db::XmlException const &e) {
        throw alpinocorpus::Error(e.what());
    }
}

/* query */
DbCorpusReaderPrivate::DbIter::DbIter(db::XmlResults const &r_)
 : r(r_)
{
}

/* end() */
DbCorpusReaderPrivate::DbIter::DbIter(db::XmlManager &mgr)
 : r(mgr.createResults())   // builds empty XmlResults
{
}

/* operator* */
std::string DbCorpusReaderPrivate::DbIter::current() const
{
    db::XmlDocument doc;
    r.peek(doc);
    return doc.getName();
}

/* operator== */
bool DbCorpusReaderPrivate::DbIter::equals(IterImpl const &that) const
{
    try {
        // The const_casts are needed because hasNext() is not const.
        // XXX should be safe.
        DbIter &other= const_cast<DbIter&>(dynamic_cast<DbIter const &>(that));
        DbIter &self = const_cast<DbIter&>(*this);
        try {
            if (!self.r.hasNext() && !other.r.hasNext())
                return true;        // both at end()
        } catch (db::XmlException const &e) {
            throw alpinocorpus::Error(e.what());
        }
        return self.r == other.r;
    } catch (std::bad_cast const &e) {
        return false;
    }
}

/* operator++ */
void DbCorpusReaderPrivate::DbIter::next()
{
    try {
        db::XmlDocument doc;
        r.next(doc);
    } catch (db::XmlException const &e) {
        throw alpinocorpus::Error(e.what());
    }
}

DbCorpusReaderPrivate::QueryIter::QueryIter(db::XmlResults const &r)
 : DbIter(r)
{
}

std::string DbCorpusReaderPrivate::QueryIter::contents(CorpusReader const &) const
{
    db::XmlValue v;
    r.peek(v);
    return v.getNodeValue();
}
    
CorpusReader::EntryIterator DbCorpusReader::getBegin() const
{
    return d_private->getBegin();
}

CorpusReader::EntryIterator DbCorpusReader::getEnd() const
{
    return d_private->getEnd();
}

std::string DbCorpusReader::getName() const
{
  return d_private->getName();
}

size_t DbCorpusReader::getSize() const
{
    return d_private->getSize();
}
    
std::string DbCorpusReader::readEntry(std::string const &entry) const
{
    return d_private->readEntry(entry);
}
    
std::string DbCorpusReader::readEntryMarkQueries(std::string const &entry, 
    std::list<MarkerQuery> const &queries) const
{
    return d_private->readEntryMarkQueries(entry, queries);
}

CorpusReader::EntryIterator DbCorpusReader::runXPath(std::string const &query) const
{
    return d_private->runXPath(query);
}

CorpusReader::EntryIterator DbCorpusReader::runXQuery(std::string const &query) const
{
    return d_private->runXQuery(query);
}
    
DbCorpusReaderPrivate::DbCorpusReaderPrivate(std::string const &path)
 : mgr(), container()
{
    try {
        db::XmlContainerConfig config;
        config.setReadOnly(true);
        container = mgr.openContainer(path, config);
        // Nasty: using a hard-coded alias to work use in the xpath queries.
        container.addAlias("corpus"); 
        setNameAndCollection(path);
    } catch (db::XmlException const &e) {
        throw OpenError(path, e.what());
    }
}

DbCorpusReaderPrivate::~DbCorpusReaderPrivate()
{
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::getBegin() const
{
    return EntryIterator(new DbIter(container));
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::getEnd() const
{
    return EntryIterator(new DbIter(mgr));
}

std::string DbCorpusReaderPrivate::getName() const
{
    return container.getName();
}

std::string DbCorpusReaderPrivate::readEntry(std::string const &filename) const
{
    try {
        db::XmlDocument doc(container.getDocument(filename, db::DBXML_LAZY_DOCS));
        std::string content;
        return doc.getContent(content);

    } catch (db::XmlException const &e) {
        std::ostringstream msg;
        msg << "entry \""                  << filename
            << "\" cannot be read from \"" << container.getName()
            << "\" ("                      << e.what()
            << ")";
        throw Error(msg.str());
    }
}
    
std::string DbCorpusReaderPrivate::readEntryMarkQueries(std::string const &entry,
    std::list<MarkerQuery> const &queries) const
{
    std::string content;
    
    try {
        db::XmlDocument doc(container.getDocument(entry, db::DBXML_LAZY_DOCS));
        doc.getContent(content);
    } catch (db::XmlException const &e) {
        std::ostringstream msg;
        msg << "entry \""                  << entry
        << "\" cannot be read from \"" << container.getName()
        << "\" ("                      << e.what()
        << ")";
        throw Error(msg.str());
    }

    return markQueries(content, queries);
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::runXPath(std::string const &query) const
{
    return runXQuery(std::string("collection('corpus')" + query));
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::runXQuery(std::string const &query)
    const
{
    // XXX use DBXML_DOCUMENT_PROJECTION and return to whole-doc containers?

    try {
        db::XmlQueryContext ctx
            = mgr.createQueryContext(db::XmlQueryContext::LiveValues,
                                     db::XmlQueryContext::Lazy);
        ctx.setDefaultCollection(collection);
        db::XmlResults r(mgr.query(query, ctx,
                                     db::DBXML_LAZY_DOCS
                                   | db::DBXML_WELL_FORMED_ONLY
                                  ));
        return EntryIterator(new QueryIter(r));
    } catch (db::XmlException const &e) {
        throw alpinocorpus::Error(e.what());
    }
}

/*
 * Set corpus name to container name; set collection to a usable collection
 * name.
 *
 * The collection name is used for querying. We set it to the absolute path
 * so we can still run queries after a chdir().
 * For some reason, DB XML strips off a leading slash in the filename,
 * so we prepend an extra one.
 */
void DbCorpusReaderPrivate::setNameAndCollection(std::string const &path)
{
    std::string uri = "/" + name();
    collection = util::toPercentEncoding(uri);
}

}   // namespace alpinocorpus
