/*
 * Oracle Berkeley DB XML-based treebank reader.
 * Written by Lars Buitinck.
 *
 * The basic format is explained in the header. An additional notes:
 *  - We use DBML_WELL_FORMED_ONLY to prevent having to read the DTD.
 *    This only works because the DTD does not define entities.
 */

#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include <QString>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <iostream>

namespace db = DbXml;

namespace {
    QString toQString(std::string const &s)
    {
        return QString::fromUtf8(s.c_str());
    }
}

namespace alpinocorpus {

/* begin() */
DbCorpusReader::DbIter::DbIter(db::XmlContainer &container)
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
DbCorpusReader::DbIter::DbIter(db::XmlResults const &r_)
 : r(r_)
{
}

/* end() */
DbCorpusReader::DbIter::DbIter(db::XmlManager &mgr)
 : r(mgr.createResults())   // builds empty XmlResults
{
}

/* operator* */
QString DbCorpusReader::DbIter::current() const
{
    db::XmlDocument doc;
    r.peek(doc);
    return toQString(doc.getName());
}

/* operator== */
bool DbCorpusReader::DbIter::equals(IterImpl const *that) const
{
    try {
        // The const_casts are needed because hasNext() is not const.
        // XXX should be safe.
        DbIter &other= const_cast<DbIter&>(dynamic_cast<DbIter const &>(*that));
        DbIter &self = const_cast<DbIter&>(*this);
        if (!self.r.hasNext() && !other.r.hasNext())
            return true;        // both at end()
        return self.r == other.r;
    } catch (std::bad_cast const &e) {
        return false;
    }
}

/* operator++ */
void DbCorpusReader::DbIter::next()
{
    try {
        db::XmlDocument doc;
        r.next(doc);
    } catch (db::XmlException const &e) {
        throw alpinocorpus::Error(e.what());
    }
}

DbCorpusReader::QueryIter::QueryIter(db::XmlResults const &r)
 : DbIter(r)
{
}

QString DbCorpusReader::QueryIter::contents(CorpusReader const &) const
{
    db::XmlValue v;
    r.peek(v);
    return toQString(v.getNodeValue());
}

DbCorpusReader::DbCorpusReader(QString const &qpath)
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

DbCorpusReader::~DbCorpusReader()
{
}

CorpusReader::EntryIterator DbCorpusReader::getBegin() const
{
    return EntryIterator(new DbIter(container));
}

CorpusReader::EntryIterator DbCorpusReader::getEnd() const
{
    return EntryIterator(new DbIter(mgr));
}

QString DbCorpusReader::readEntry(QString const &filename) const
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

CorpusReader::EntryIterator DbCorpusReader::runXPath(QString const &query) const
{
    return runXQuery(QString("collection('corpus')" + query));
}

CorpusReader::EntryIterator DbCorpusReader::runXQuery(QString const &query)
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
void DbCorpusReader::setNameAndCollection(QString const &path)
{
    //collection = QFileInfo(path).absoluteFilePath().toLocal8Bit().data();

    setName(toQString(container.getName()));
    
    collection = std::string("/") + name().toLocal8Bit().data();
}

}   // namespace alpinocorpus
