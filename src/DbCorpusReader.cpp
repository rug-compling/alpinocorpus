#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include <QString>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace db = DbXml;

namespace {
    const u_int32_t FLAGS = db::DBXML_LAZY_DOCS | db::DBXML_WELL_FORMED_ONLY;

    /* We handle Unicode strings exclusively */
    std::string toStdString(QString const &s)
    {
        return std::string(s.toUtf8().constData());
    }
    QString toQString(char const *s) { return QString::fromUtf8(s); }
    QString toQString(std::string const &s) { return toQString(c.c_str()); }
}

namespace alpinocorpus {

/* end() */
DbCorpusReader::DbIter::DbIter(db::XmlManager &mgr)
 : r(mgr.createResults())   // builds empty XmlResults
{
}

/* begin(), query() */
DbCorpusReader::DbIter::DbIter(db::XmlResults &r_)
 : r(r_)
{
    try {
        db::XmlDocument doc;
        r.peek(doc);
        cur = toQString(doc.getName());
    } catch (db::XmlException const &e) {
        throw Error(e.what());
    }
}

/* operator* */
QString const &DbCorpusReader::DbIter::current() const { return cur; }

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
    }
    catch (std::bad_cast const &e) {
        return false;
    }
}

/* operator++ */
void DbCorpusReader::DbIter::next()
{
    try {
        db::XmlDocument doc;
        r.next(doc);
        cur = toQString(doc.getName());
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
    } catch (db::XmlException const &e) {
        throw OpenError(qpath, toQString(e.what()));
    }
}

DbCorpusReader::~DbCorpusReader()
{
}

CorpusReader::EntryIterator DbCorpusReader::begin() const
{
    try {
        // The const_cast below should be safe due to locking.
        // XXX double-check this
        db::XmlContainer &cont(const_cast<db::XmlContainer &>(container));

        db::XmlResults r(cont.getAllDocuments(FLAGS));
        return EntryIterator(new DbIter(r));
    }
    catch (db::XmlException const &e) {
        throw alpinocorpus::Error(e.what());
    }
}

CorpusReader::EntryIterator DbCorpusReader::end() const
{
    return EntryIterator(new DbIter(const_cast<db::XmlManager &>(mgr)));
}

QVector<QString> DbCorpusReader::entries() const
{
    QVector<QString> ents;
    try {
        for (EntryIterator i(begin()), to(end()); i != to; ++i)
            ents.push_back(*i);
        return ents;
    }
    catch (db::XmlException const &e) {
        throw Error(e.what());
    }
}

QString DbCorpusReader::name() const
{
    return toQString(container.getName());
}

EntryIterator DbCorpusReader::query(QString const &qq) const
{
    try {
        // The const_cast below should be safe due to locking.
        // XXX double-check this
        db::XmlManager &m(const_cast<db::XmlManager &>(mgr));

        db::XmlQueryContext ctx;
        return EntryIterator(new DbIter(mgr.query(toStdString(q), ctx, FLAGS)));
    }
    catch (db::XmlException const &e) {
        throw Error(e.what());
    }
}

QString DbCorpusReader::read(QString const &filename)
{
    std::string name(toStdString(filename));

    try {
        db::XmlDocument doc(container.getDocument(name, db::DBXML_LAZY_DOCS));
        std::string content;
        return toQString(doc.getContent(content));
    }
    catch (db::XmlException const &e) {
        std::ostringstream msg;
        msg << "entry \""                  << name
            << "\" cannot be read from \"" << container.getName()
            << "\" ("                      << e.what()
            << ")";
        throw Error(msg.str());
    }
}

}   // namespace alpinocorpus
