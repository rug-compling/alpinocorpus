#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>
#include <QDir>

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
        // but they don't have a no-throw guarantee either.
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
        db::XmlUpdateContext ctx(mkUpdateContext());
        for (CorpusReader::EntryIterator i(corpus.begin()), end(corpus.end());
             i != end; ++i)
            write(*i, corpus.read(*i), ctx);
    }

    void DbCorpusWriter::write(QString const &name, QString const &content,
                               XmlUpdateContext &ctx)
    {
        try {
            std::string canonical(QDir::fromNativeSeparators(name)
                                  .toUtf8()
                                  .data());
            container.putDocument(name, content, ctx, db::WELL_FORMED_ONLY);
        } catch (XmlException const &e) {
            std::ostringstream msg;
            msg << "cannot write document \"" << name
                << "\": " << e.what();
            throw Error(msg.str());
        }
    }
}
