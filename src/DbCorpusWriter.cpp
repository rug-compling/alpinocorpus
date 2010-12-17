#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>
#include <QDir>
#include <sstream>

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
        } catch (db::XmlException const &e) {
            throw OpenError(qpath, e.what());
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
            std::string msg("cannot create XML database update context: ");
            msg += e.what();
            throw Error(msg);
        }
    }

    void DbCorpusWriter::write(QString const &name, QString const &content)
    {
        db::XmlUpdateContext ctx;
        write(name, content, mkUpdateContext(ctx));
    }

    void DbCorpusWriter::write(CorpusReader &corpus)
    {
        db::XmlUpdateContext ctx;
        mkUpdateContext(ctx);
        for (CorpusReader::EntryIterator i(corpus.begin()), end(corpus.end());
             i != end; ++i)
            write(*i, corpus.read(*i), ctx);
    }

    /*
     * Transforms content to UTF-8.
     * XXX: check for/rewrite/remove encoding in XML document?
     */
    void DbCorpusWriter::write(QString const &name, QString const &content,
                               db::XmlUpdateContext &ctx)
    {
        try {
            std::string canonical(QDir::fromNativeSeparators(name)
                                  .toUtf8()
                                  .data());
            container.putDocument(canonical, content.toUtf8().data(), ctx,
                                  db::DBXML_WELL_FORMED_ONLY);
        } catch (db::XmlException const &e) {
            std::ostringstream msg;
            msg << "cannot write document \"" << name.toLocal8Bit().data()
                << "\": " << e.what();
            throw Error(msg.str());
        }
    }
}
