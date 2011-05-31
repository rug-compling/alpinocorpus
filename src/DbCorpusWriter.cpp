#include <QDir>
#include <QString>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sstream>

#include <dbxml/DbXml.hpp>

#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

namespace db = DbXml;

namespace alpinocorpus {
    class DbCorpusWriterPrivate : public util::NonCopyable
    {
    public:
        /** Open path for writing. */
        DbCorpusWriterPrivate(QString const &path, bool overwrite);
        DbXml::XmlUpdateContext &mkUpdateContext(DbXml::XmlUpdateContext &);
        void write(QString const &, QString const &, DbXml::XmlUpdateContext &);
        void writeFailFirst(CorpusReader const &, DbXml::XmlUpdateContext &);
        void writeFailSafe(CorpusReader const &, DbXml::XmlUpdateContext &);
    private:
        DbXml::XmlManager d_mgr;
        DbXml::XmlContainer d_container;        

    };
    
    DbCorpusWriter::DbCorpusWriter(QString const &path, bool overwrite)
        : d_private(new DbCorpusWriterPrivate(path, overwrite))
    {}
    
    DbCorpusWriter::~DbCorpusWriter()
    {}
    
    void DbCorpusWriter::writeEntry(QString const &name, QString const &content)
    {
        db::XmlUpdateContext ctx;
        d_private->write(name, content, d_private->mkUpdateContext(ctx));
    }
    
    void DbCorpusWriter::writeEntry(CorpusReader const &corpus, bool fail_first)
    {
        db::XmlUpdateContext ctx;
        d_private->mkUpdateContext(ctx);
        
        if (fail_first)
            d_private->writeFailFirst(corpus, ctx);
        else
            d_private->writeFailSafe(corpus, ctx);
    }
    
    DbCorpusWriterPrivate::DbCorpusWriterPrivate(QString const &qpath, bool overwrite)
        : d_mgr(), d_container()
    {
        try {
            db::XmlContainerConfig config;
            config.setReadOnly(false);

            std::string path(qpath.toLocal8Bit().data());

            if (overwrite) {
                if (std::remove(path.c_str()) != 0 && errno != ENOENT)
                    throw OpenError(qpath,
                                    QString("cannot remove file: %1")
                                        .arg(std::strerror(errno)));
                d_container = d_mgr.createContainer(path, config,
                                                    db::XmlContainer
                                                    ::NodeContainer);
            } else
                d_container = d_mgr.openContainer(path, config);
        } catch (db::XmlException const &e) {
            throw OpenError(qpath, e.what());
        }
    }

    db::XmlUpdateContext &DbCorpusWriterPrivate::mkUpdateContext(
        db::XmlUpdateContext &ctx)
    {
        // Note: this function may be unnecessary; no exceptions are listed
        // for XmlManager::createUpdateContext() or the copy constructor,
        // but they don't have a no-throw guarantee either.
        try {
            return ctx = d_mgr.createUpdateContext();
        } catch (db::XmlException const &e) {
            std::string msg("cannot create XML database update context: ");
            msg += e.what();
            throw Error(msg);
        }
    }

    void DbCorpusWriterPrivate::writeFailFirst(CorpusReader const &corpus,
        db::XmlUpdateContext &ctx)
    {
        for (CorpusReader::EntryIterator i(corpus.begin()), end(corpus.end());
             i != end; ++i)
            write(*i, corpus.read(*i), ctx);
    }

    void DbCorpusWriterPrivate::writeFailSafe(CorpusReader const &corpus,
        db::XmlUpdateContext &ctx)
    {
        BatchError err;

        for (CorpusReader::EntryIterator i(corpus.begin()), end(corpus.end());
             i != end; ++i)
            try {
                write(*i, corpus.read(*i), ctx);
            } catch (Error const &e) {
                err.append(e);
            }

        if (!err.empty())
            throw err;
    }

    /*
     * Transforms content to UTF-8.
     * XXX: check for/rewrite/remove encoding in XML document?
     */
    void DbCorpusWriterPrivate::write(QString const &name, QString const &content,
                               db::XmlUpdateContext &ctx)
    {
        try {
            std::string canonical(QDir::fromNativeSeparators(name)
                                  .toUtf8()
                                  .data());
            d_container.putDocument(canonical, content.toUtf8().data(), ctx,
                                  db::DBXML_WELL_FORMED_ONLY);
        } catch (db::XmlException const &e) {
            if (e.getExceptionCode() == db::XmlException::UNIQUE_ERROR)
                throw DuplicateKey(name);
            else {
                std::ostringstream msg;
                msg << "cannot write document \"" << name.toLocal8Bit().data()
                    << "\": " << e.what();
                throw Error(msg.str());
            }
        }
    }
}
