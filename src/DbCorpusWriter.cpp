#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>

#include <dbxml/DbXml.hpp>

#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/util/NonCopyable.hh>

namespace bf = boost::filesystem;
namespace db = DbXml;

namespace alpinocorpus {
    class DbCorpusWriterPrivate : public util::NonCopyable
    {
    public:
        /** Open path for writing. */
        DbCorpusWriterPrivate(std::string const &path, bool overwrite);
        DbXml::XmlUpdateContext &mkUpdateContext(DbXml::XmlUpdateContext &);
        void write(std::string const &, std::string const &, DbXml::XmlUpdateContext &);
        void writeFailFirst(CorpusReader const &, DbXml::XmlUpdateContext &);
        void writeFailSafe(CorpusReader const &, DbXml::XmlUpdateContext &);
    private:
        DbXml::XmlManager d_mgr;
        DbXml::XmlContainer d_container;        

    };
    
    DbCorpusWriter::DbCorpusWriter(std::string const &path, bool overwrite)
        : d_private(new DbCorpusWriterPrivate(path, overwrite))
    {}
    
    DbCorpusWriter::~DbCorpusWriter()
    {
        delete d_private;
    }
    
    void DbCorpusWriter::writeEntry(std::string const &name, std::string const &content)
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
    
    DbCorpusWriterPrivate::DbCorpusWriterPrivate(std::string const &path, bool overwrite)
        : d_mgr(), d_container()
    {
        try {
            db::XmlContainerConfig config;
            config.setReadOnly(false);


            if (overwrite) {
                if (std::remove(path.c_str()) != 0 && errno != ENOENT)
                    throw OpenError(path,
                                    std::string("cannot remove file: ") +
                                    std::strerror(errno));
                d_container = d_mgr.createContainer(path, config,
                                                    db::XmlContainer
                                                    ::NodeContainer);
            } else
                d_container = d_mgr.openContainer(path, config);
        } catch (db::XmlException const &e) {
            throw OpenError(path, e.what());
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
        CorpusReader::EntryIterator i = corpus.entries();
        while (i.hasNext())
        {
            Entry entry = i.next(corpus);
            write(entry.name, corpus.read(entry.name), ctx);
        }
    }

    void DbCorpusWriterPrivate::writeFailSafe(CorpusReader const &corpus,
        db::XmlUpdateContext &ctx)
    {
        BatchError err;

        CorpusReader::EntryIterator i = corpus.entries();
        while (i.hasNext())
        {
            Entry entry = i.next(corpus);
            try {
                write(entry.name, corpus.read(entry.name), ctx);
            } catch (Error const &e) {
                err.append(e);
            }
        }

        if (!err.empty())
            throw err;
    }

    /*
     * Transforms content to UTF-8.
     * XXX: check for/rewrite/remove encoding in XML document?
     */
    void DbCorpusWriterPrivate::write(std::string const &name, std::string const &content,
                               db::XmlUpdateContext &ctx)
    {
        try {
            std::string canonical(bf::path(name).generic_string());
            d_container.putDocument(canonical, content, ctx,
                                  db::DBXML_WELL_FORMED_ONLY);
        } catch (db::XmlException const &e) {
            if (e.getExceptionCode() == db::XmlException::UNIQUE_ERROR)
                throw DuplicateKey(name);
            else {
                std::ostringstream msg;
                msg << "cannot write document \"" << name << "\": " << e.what();
                throw Error(msg.str());
            }
        }
    }
}
