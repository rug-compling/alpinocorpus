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

    class DbIterator : public alpinocorpus::CorpusReader::EntryIterator
    {
        QString cur;
        db::XmlResults r;

      public:
        /* begin() */
        DbIterator(db::XmlContainer &container)
        {
            try {
                r = container.getAllDocuments( db::DBXML_LAZY_DOCS
                                             | db::DBXML_WELL_FORMED_ONLY
                                             );
            } catch (db::XmlException const &e) {
                throw alpinocorpus::Error(e.what());
            }
        }

        /* end() */
        DbIterator(db::XmlManager &mgr)
         : r(mgr.createResults())   // builds empty XmlResults
        {
        }

      private:
        /* copy constructor, operator= */
        void copy(EntryIterator const &other)
        {
            if (&other != this) {
                DbIterator const &i = dynamic_cast<DbIterator const &>(other);
                r   = i.r;
                cur = i.cur;
            }
        }

        /* operator* */
        value_type &current() { return cur; }

        /* operator== */
        bool equals(EntryIterator const &other)
        {
            try {
                DbIterator &i
                  = const_cast<DbIterator &>(
                      dynamic_cast<DbIterator const &>(other));
                if (!r.hasNext() && !i.r.hasNext())
                    return true;        // both at end()
                return r == i.r;
            } catch (std::bad_cast const &e) {
                return false;
            }
        }

        /* operator++ */
        void next()
        {
            try {
                db::XmlDocument doc;
                r.next(doc);
                cur = toQString(doc.getName());
            } catch (db::XmlException const &e) {
                throw alpinocorpus::Error(e.what());
            }
        }
    };
}

namespace alpinocorpus {

DbCorpusReader::DbCorpusReader(QString const &qpath)
 : mgr(), container()
{
    std::string path(qpath.toLocal8Bit().data());

    try {
        db::XmlContainerConfig config;
        config.setReadOnly(true);
        container = mgr.openContainer(path, config);
    } catch (db::XmlException const &e) {
        throw OpenError(qpath, QString::fromUtf8(e.what()));
    }
}

DbCorpusReader::~DbCorpusReader()
{
}

CorpusReader::EntryIterator DbCorpusReader::begin() const
{
    return DbIterator(const_cast<db::XmlContainer &>(container));
}

CorpusReader::EntryIterator DbCorpusReader::end() const
{
    return DbIterator(const_cast<db::XmlManager &>(mgr));
}

QVector<QString> DbCorpusReader::entries() const
{
    QVector<QString> ents;
    try {
        // The const_cast below should be safe due to locking.
        // XXX double-check this
        db::XmlContainer &container(const_cast<db::XmlContainer &>(this->container));
        db::XmlResults r(container.getAllDocuments( db::DBXML_LAZY_DOCS
                                                  | db::DBXML_WELL_FORMED_ONLY
                                                  ));

        for (db::XmlDocument doc; r.next(doc); )
            ents.push_back(toQString(doc.getName()));
        return ents;
    } catch (db::XmlException const &e) {
        throw Error(e.what());
    }
}

QString DbCorpusReader::name() const
{
    return toQString(container.getName());
}

QString DbCorpusReader::read(QString const &filename)
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

}   // namespace alpinocorpus
