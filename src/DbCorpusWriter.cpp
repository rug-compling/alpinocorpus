#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>

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

    void DbCorpusWriter::write(QString const &name, QString const &content)
    {
        write(name, content, mgr.createUpdateContext());
    }

    void DbCorpusWriter::write(CorpusReader const &corpus)
    {
        QVector<QString> entry(corpus.entries);
        db::XmlUpdateContext ctx(mgr.createUpdateContext());
        for (size_t i=0; i<entry.size(); i++)
            write(entry[i], corpus.read(entry[i]), ctx);
    }

    void DbCorpusWriter::write(QString const &name, QString const &content,
                               XmlUpdateContext &ctx)
    {
        // Note: we handle errors here under the assumption that
        // XmlManager::createUpdateContext() cannot fail.
        // No exceptions are listed in its documentation, but it doesn't have
        // no-throw guarantee either.
        try {
            container.putDocument(name, content, ctx, db::WELL_FORMED_ONLY);
        } catch (XmlException const &e) {
            std::ostringstream msg;
            msg << "cannot write document \"" << name
                << "\": " << e.what();
            throw Error(msg.str());
        }
    }
}
