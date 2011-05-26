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

#include <QString>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <iostream>

#include <dbxml/DbXml.hpp>

namespace db = DbXml;

namespace {
    QString toQString(std::string const &s)
    {
        return QString::fromUtf8(s.c_str());
    }
}

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
        
        QString current() const;
        bool equals(IterImpl const &) const;
        void next();
        
    protected:
        mutable DbXml::XmlResults r;
        
        DbIter(DbXml::XmlResults const &);
    };
    
    struct QueryIter : public DbIter
    {
        QueryIter(DbXml::XmlResults const &);
        QString contents(CorpusReader const &) const;
    };
    
public:
    DbCorpusReaderPrivate(QString const &);
    ~DbCorpusReaderPrivate();
    EntryIterator getBegin() const;
    EntryIterator getEnd() const;
    size_t getSize() const
    {
        return const_cast<DbXml::XmlContainer &>(container).getNumDocuments();
    }
    bool validQuery(QueryDialect d, bool variables, QString const &query) const;
    QString readEntry(QString const &) const;
    EntryIterator runXPath(QString const &) const;
    EntryIterator runXQuery(QString const &) const;
    
private:
    void setNameAndCollection(QString const &);
    
};
    
DbCorpusReader::DbCorpusReader(QString const &name) :
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
QString DbCorpusReaderPrivate::DbIter::current() const
{
    db::XmlDocument doc;
    r.peek(doc);
    return toQString(doc.getName());
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

QString DbCorpusReaderPrivate::QueryIter::contents(CorpusReader const &) const
{
    db::XmlValue v;
    r.peek(v);
    return toQString(v.getNodeValue());
}
    
CorpusReader::EntryIterator DbCorpusReader::getBegin() const
{
    return d_private->getBegin();
}

CorpusReader::EntryIterator DbCorpusReader::getEnd() const
{
    return d_private->getEnd();
}

size_t DbCorpusReader::getSize() const
{
    return d_private->getSize();
}
    
bool DbCorpusReader::validQuery(QueryDialect d, bool variables, QString const &query) const
{
    return d_private->isValidQuery(d, variables, query);
}


QString DbCorpusReader::readEntry(const QString &entry) const
{
    return d_private->readEntry(entry);
}

CorpusReader::EntryIterator DbCorpusReader::runXPath(QString const &query) const
{
    return d_private->runXPath(query);
}

CorpusReader::EntryIterator DbCorpusReader::runXQuery(QString const &query) const
{
    return d_private->runXQuery(query);
}
    
DbCorpusReaderPrivate::DbCorpusReaderPrivate(QString const &qpath)
 : mgr(), container()
{
    std::string path(qpath.toLocal8Bit().data());

    try {
        db::XmlContainerConfig config;
        config.setReadOnly(true);
        container = mgr.openContainer(path, config);
        // Nasty: using a hard-coded alias to work use in the xpath queries.
        container.addAlias("corpus"); 
        setNameAndCollection(qpath);
    } catch (db::XmlException const &e) {
        throw OpenError(qpath, QString::fromUtf8(e.what()));
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

bool DbCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, QString const &query) const
{
    try {
        db::XmlQueryContext ctx = mgr.createQueryContext();
        QByteArray queryData(query.toUtf8());
        mgr.prepare(queryData.constData(), ctx);
    } catch (db::XmlException const &e) {
        return false;
    }
    
    return true;
}


QString DbCorpusReaderPrivate::readEntry(QString const &filename) const
{
    std::string name(filename.toUtf8().data());

    try {
        db::XmlDocument doc(container.getDocument(name, db::DBXML_LAZY_DOCS));
        std::string content;
        return toQString(doc.getContent(content));

    } catch (db::XmlException const &e) {
        std::ostringstream msg;
        msg << "entry \""                  << name
            << "\" cannot be read from \"" << container.getName()
            << "\" ("                      << e.what()
            << ")";
        throw Error(msg.str());
    }
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::runXPath(QString const &query) const
{
    return runXQuery(QString("collection('corpus')" + query));
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::runXQuery(QString const &query)
    const
{
    // XXX use DBXML_DOCUMENT_PROJECTION and return to whole-doc containers?

    try {
        db::XmlQueryContext ctx
            = mgr.createQueryContext(db::XmlQueryContext::LiveValues,
                                     db::XmlQueryContext::Lazy);
        ctx.setDefaultCollection(collection);
        db::XmlResults r(mgr.query(query.toUtf8().data(), ctx,
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
void DbCorpusReaderPrivate::setNameAndCollection(QString const &path)
{
    //collection = QFileInfo(path).absoluteFilePath().toLocal8Bit().data();

    setName(toQString(container.getName()));
    
    collection = std::string("/") + name().toLocal8Bit().data();
}

}   // namespace alpinocorpus
