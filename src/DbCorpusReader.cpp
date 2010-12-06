#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include <QString>
#include <sstream>
#include <stdexcept>
#include <string>

namespace db = DbXml;

namespace {
    QString toQString(std::string const &s)
    {
        return QString::fromUtf8(s.c_str());
    }
}

namespace alpinocorpus {

DbCorpusReader::DbCorpusReader(QString const &path)
 : mgr(), container(), qpath(path)
{
}

DbCorpusReader::~DbCorpusReader()
{
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

bool DbCorpusReader::open()
{
    std::string path(qpath.toLocal8Bit().data());

    try {
        db::XmlContainerConfig config;
        config.setReadOnly(true);
        container = mgr.openContainer(path, config);
    } catch (db::XmlException const &e) {
        return false;
    }

    return true;
}

QString DbCorpusReader::read(QString const &filename)
{
    std::string name(filename.toUtf8().data());

    try {
        db::XmlDocument doc(container.getDocument(name, db::DBXML_LAZY_DOCS));
        std::string content;
        // FIXME: this interprets UTF-8 as ASCII
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
