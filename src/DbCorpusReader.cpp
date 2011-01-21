#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include <QString>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

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
    const_cast<db::XmlResults &>(r).peek(doc);
    return toQString(doc.getName());
}

/* operator== */
bool DbCorpusReader::DbIter::equals(IterImpl const *that) const
{
    try {
        // The const_casts are needed because hasNext() is not const.
        // XXX should be safe.
        DbIter *other = const_cast<DbIter*>(dynamic_cast<DbIter const *>(that));
        DbIter &self  = const_cast<DbIter&>(*this);
        if (!self.r.hasNext() && !other->r.hasNext())
            return true;        // both at end()
        return self.r == other->r;
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

DbCorpusReader::DbCorpusReader(QString const &qpath)
 : mgr(), container()
{
    std::string path(qpath.toLocal8Bit().data());

    try {
        db::XmlContainerConfig config;
        config.setReadOnly(true);
        container = mgr.openContainer(path, config);
        setName(toQString(container.getName()));
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

CorpusReader::EntryIterator DbCorpusReader::runQuery(QString const &query) const
{
    db::XmlQueryContext ctx;    // XXX should this be in the iterator?
    // XXX use DBXML_DOCUMENT_PROJECTION and return to whole-doc containers?
    db::XmlResults r(mgr.query(query.toUtf8().data(), ctx,
                               db::DBXML_LAZY_DOCS | db::DBXML_WELL_FORMED_ONLY
                              ));

    return EntryIterator(new DbIter(r));
}

}   // namespace alpinocorpus
