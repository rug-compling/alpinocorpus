#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include <dbxml/DbXml.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IterImpl.hh>

#include "DbCorpusReaderPrivate.hh"
#include "util/url.hh"

namespace db = DbXml;

namespace alpinocorpus {

/* begin() */
DbCorpusReaderPrivate::DbIter::DbIter(db::XmlContainer &container)
{
    try {
        r = container.getAllDocuments( db::DBXML_LAZY_DOCS
                                     | db::DBXML_WELL_FORMED_ONLY
                                     );
    } catch (db::XmlException const &e) {
        throw Error(e.what());
    }
}

/* query */
DbCorpusReaderPrivate::DbIter::DbIter(db::XmlResults const &r_)
 : r(r_)
{
}

/* end() */
DbCorpusReaderPrivate::DbIter::DbIter(db::XmlManager &mgr)
 : r(mgr.createResults())   // builds empty XmlResults
{
}

IterImpl *DbCorpusReaderPrivate::DbIter::copy() const
{
    // XXX - Copy constructor of XmlResults copies handle but not body.
    //       The copyResults() method returns an XmlResults instance that
    //       is eagerly evaluated. Is there a way to copy XmlResults,
    //       retain the iterator position, and have it lazy?

    // No pointer members
    return new DbIter(*this);
}

bool DbCorpusReaderPrivate::DbIter::hasNext()
{
  try {
      return r.hasNext();
    } catch (db::XmlException const &e) {
        if (e.getExceptionCode() == db::XmlException::OPERATION_INTERRUPTED)
          throw IterationInterrupted();
        else
          throw Error(e.what());
    }
}

/* operator++ */
Entry DbCorpusReaderPrivate::DbIter::next(CorpusReader const &)
{
    db::XmlValue v;

    try {
        r.next(v);
    } catch (db::XmlException const &e) {
        if (e.getExceptionCode() == db::XmlException::OPERATION_INTERRUPTED)
          throw IterationInterrupted();
        else
          throw Error(e.what());
    }
   
    std::string name;
    std::string value;

    if (v.isNode()) {
        try {
            db::XmlDocument doc = v.asDocument();
            name = doc.getName();
        } catch (db::XmlException &) {
          // Could not use node as a document. Why is there no isDocument()
          // method?
        }
    }

    value = v.asString();

    Entry e = {name, value};

    return e;
}

DbCorpusReaderPrivate::QueryIter::QueryIter(db::XmlResults const &r, db::XmlQueryContext const &ctx)
 : DbIter(r), context(ctx)
{
}

void DbCorpusReaderPrivate::QueryIter::interrupt()
{
    context.interruptQuery();
}

IterImpl *DbCorpusReaderPrivate::QueryIter::copy() const
{
    // XXX - See DbIter::copy()

    return new QueryIter(*this);
}
DbCorpusReaderPrivate::DbCorpusReaderPrivate(std::string const &path)
 : mgr(db::DBXML_ALLOW_EXTERNAL_ACCESS), container()
{
    try {
        db::XmlContainerConfig config;
        config.setReadOnly(true);
        container = mgr.openContainer(path, config);
        // Nasty: using a hard-coded alias to work use in the xpath queries.
        container.addAlias("corpus"); 
        setNameAndCollection(path);
    } catch (db::XmlException const &e) {
        throw OpenError(path, e.what());
    }
}

DbCorpusReaderPrivate::~DbCorpusReaderPrivate()
{
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::getEntries() const
{
    return EntryIterator(new DbIter(container));
}

std::string DbCorpusReaderPrivate::getName() const
{
    return container.getName();
}

bool DbCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, std::string const &query) const
{
    try {
        db::XmlQueryContext ctx = mgr.createQueryContext();
        mgr.prepare(query, ctx);
    } catch (db::XmlException const &e) {
        return false;
    }
    
    return true;
}


std::string DbCorpusReaderPrivate::readEntry(std::string const &filename) const
{
    try {
        db::XmlDocument doc(container.getDocument(filename, db::DBXML_LAZY_DOCS));
        std::string content;
        return doc.getContent(content);

    } catch (db::XmlException const &e) {
        std::ostringstream msg;
        msg << "entry \""                  << filename
            << "\" cannot be read from \"" << container.getName()
            << "\" ("                      << e.what()
            << ")";
        throw Error(msg.str());
    }
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::runXPath(std::string const &query) const
{
    return runXQuery(std::string("collection('corpus')" + query));
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::runXQuery(std::string const &query)
    const
{
    // XXX use DBXML_DOCUMENT_PROJECTION and return to whole-doc containers?

    try {
        db::XmlQueryContext ctx
            = mgr.createQueryContext(db::XmlQueryContext::LiveValues,
                                     db::XmlQueryContext::Lazy);
        ctx.setDefaultCollection(collection);
        db::XmlResults r(mgr.query(query, ctx,
                                     db::DBXML_LAZY_DOCS
                                   | db::DBXML_WELL_FORMED_ONLY
                                  ));
        return EntryIterator(new QueryIter(r, ctx));
    } catch (db::XmlException const &e) {
        throw Error(e.what());
    }
}

/*
 * Set corpus name to container name; set collection to a usable collection
 * name.
 *
 * The collection name is used for querying. We set it to the absolute path
 * so we can still run queries after a chdir().
 * For some reason, DB XML strips off a leading slash in the filename,
 * so we prepend an extra one.
 */
void DbCorpusReaderPrivate::setNameAndCollection(std::string const &path)
{
    std::string uri = "/" + name();
    collection = util::toPercentEncoding(uri);
}

}
