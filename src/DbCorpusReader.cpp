#include <AlpinoCorpus/DbCorpusReader.hh>

#include <sstream>
#include <stdexcept>
#include <string>

namespace db = DbXml;

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
    // TODO: catch exceptions

    QVector<QString> ents;
    try {
        db::XmlContainer &container(const_cast<db::XmlContainer &>(this->container));
        db::XmlResults r(container.getAllDocuments( db::DBXML_LAZY_DOCS
                                                  | db::DBXML_WELL_FORMED_ONLY
                                                  ));

        for (db::XmlDocument doc; r.next(doc); )
            ents.push_back(QString::fromUtf8(doc.getName().c_str()));
        return ents;
    } catch (db::XmlException const &e) {
        throw std::runtime_error(e.what());
    }
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
        return QString::fromUtf8(doc.getContent(content).c_str());

    } catch (db::XmlException const &e) {
        std::ostringstream msg;
        msg << "entry \""                  << name
            << "\" cannot be read from \"" << container.getName()
            << "\" ("                      << e.what()
            << ")";
        throw std::runtime_error(msg.str());
    }
}

}   // namespace alpinocorpus
