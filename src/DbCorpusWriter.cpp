#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>
#include <QtGlobal>

namespace db = DbXml;

namespace alpinocorpus {
    DbCorpusWriter::DbCorpusWriter(QString const &qpath, bool overwrite)
     : mgr(), container()
    {
        try {
            db::XmlContainerConfig config;
            config.setReadOnly(false);

            std::string path(qpath.toLocal8Bit().data());

            if (overwrite)
                container = mgr.createContainer(path, config);
            else
                container = mgr.openContainer(path, config);
        } catch (XmlException const &e) {
            throw OpenError(e.what());
        }
    }

    db::XmlUpdateContext &DbCorpusWriter::mkUpdateContext(
                                            db::XmlUpdateContext &ctx)
    {
        // Note: this function may be unnecessary; no exceptions are listed
        // for XmlManager::createUpdateContext() or the copy constructor,
        // but the don't have no-throw guarantee either.
        try {
            return ctx = mgr.createUpdateContext();
        } catch (db::XmlException const &e) {
            std::ostringstream msg;
            msg << "cannot create XML database update context \"" << name
                << "\": " << e.what();
            throw Error(msg.str());
        }
    }

    void DbCorpusWriter::write(QString const &name, QString const &content)
    {
        write(name, content, mkUpdateContext());
    }

    void DbCorpusWriter::write(CorpusReader const &corpus)
    {
        QVector<QString> entry(corpus.entries);
        db::XmlUpdateContext ctx(mkUpdateContext());
        for (size_t i=0; i<entry.size(); i++)
            write(entry[i], corpus.read(entry[i]), ctx);
    }

    void DbCorpusWriter::write(QString const &name, QString const &content,
                               XmlUpdateContext &ctx)
    {
        try {
            std::string canonical(name.fromNativeSeparators().toUtf8().data());
            container.putDocument(name, content, ctx, db::WELL_FORMED_ONLY);
        } catch (XmlException const &e) {
            std::ostringstream msg;
            msg << "cannot write document \"" << name
                << "\": " << e.what();
            throw Error(msg.str());
        }
    }
}
