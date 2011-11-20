#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <iostream>

#include <dbxml/DbXml.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

#include <xqilla/xqilla-dom3.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <util/url.hh>

#include "DbCorpusReaderPrivate.hh"


namespace db = DbXml;

namespace xerces = XERCES_CPP_NAMESPACE;

namespace alpinocorpus {

/* begin() */
DbCorpusReaderPrivate::DbIter::DbIter(db::XmlContainer &container)
{
    try {
        r = container.getAllDocuments( db::DBXML_LAZY_DOCS
                                     | db::DBXML_WELL_FORMED_ONLY
                                     );
    } catch (db::XmlException const &e) {
        throw alpinocorpus::Error(e.what());
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

CorpusReader::IterImpl *DbCorpusReaderPrivate::DbIter::copy() const
{
    // XXX - Copy constructor of XmlResults copies handle but not body.
    //       The copyResults() method retuls an XmlResults instance that
    //       is eagerly evaluated. Is there a way to copy XmlResults,
    //       retain the iterator position, and have it lazy?

    // No pointer members
    return new DbIter(*this);
}

/* operator* */
std::string DbCorpusReaderPrivate::DbIter::current() const
{
    db::XmlDocument doc;
    r.peek(doc);
    return doc.getName();
}

/* operator== */
bool DbCorpusReaderPrivate::DbIter::equals(IterImpl const &that) const
{
    try {
        // The const_casts are needed because hasNext() is not const.
        // XXX should be safe.
        DbIter &other= const_cast<DbIter&>(dynamic_cast<DbIter const &>(that));
        DbIter &self = const_cast<DbIter&>(*this);
        try {
            // XXX - Why do we check whether we are at the end first, rather than
            // comparing the XmlResult objects immediately?
            if (!self.r.hasNext() && !other.r.hasNext())
                return true;        // both at end()
        } catch (db::XmlException const &e) {
        if (e.getExceptionCode() == db::XmlException::OPERATION_INTERRUPTED)
                throw alpinocorpus::IterationInterrupted();
            else
                throw alpinocorpus::Error(e.what());
        }
        return self.r == other.r;
    } catch (std::bad_cast const &e) {
        return false;
    }
}

/* operator++ */
void DbCorpusReaderPrivate::DbIter::next()
{
    try {
        db::XmlDocument doc;
        r.next(doc);
    } catch (db::XmlException const &e) {
        if (e.getExceptionCode() == db::XmlException::OPERATION_INTERRUPTED)
          throw alpinocorpus::IterationInterrupted();
        else
          throw alpinocorpus::Error(e.what());
    }
}

DbCorpusReaderPrivate::QueryIter::QueryIter(db::XmlResults const &r, db::XmlQueryContext const &ctx)
 : DbIter(r), context(ctx)
{
}

void DbCorpusReaderPrivate::QueryIter::interrupt()
{
    context.interruptQuery();
}

std::string DbCorpusReaderPrivate::QueryIter::contents(CorpusReader const &) const
{
    db::XmlValue v;
    r.peek(v);
    return v.getNodeValue();
}

CorpusReader::IterImpl *DbCorpusReaderPrivate::QueryIter::copy() const
{
    // XXX - See DbIter::copy()

    return new QueryIter(*this);
}
DbCorpusReaderPrivate::DbCorpusReaderPrivate(std::string const &path)
 : mgr(), container()
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

CorpusReader::EntryIterator DbCorpusReaderPrivate::getBegin() const
{
    return EntryIterator(new DbIter(container));
}

CorpusReader::EntryIterator DbCorpusReaderPrivate::getEnd() const
{
    return EntryIterator(new DbIter(mgr));
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
    
std::string DbCorpusReaderPrivate::readEntryMarkQueries(std::string const &entry,
    std::list<MarkerQuery> const &queries) const
{
    std::string content;
    
    try {
        db::XmlDocument doc(container.getDocument(entry, db::DBXML_LAZY_DOCS));
        doc.getContent(content);
    } catch (db::XmlException const &e) {
        std::ostringstream msg;
        msg << "entry \""                  << entry
        << "\" cannot be read from \"" << container.getName()
        << "\" ("                      << e.what()
        << ")";
        throw Error(msg.str());
    }

    // Prepare the DOM parser.
    xerces::DOMImplementation *xqillaImplementation =
        xerces::DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));
    AutoRelease<xerces::DOMLSParser> parser(xqillaImplementation->createLSParser(
        xerces::DOMImplementationLS::MODE_SYNCHRONOUS, 0));
    
    // Parse the document.
    xerces::MemBufInputSource xmlInput(reinterpret_cast<XMLByte const *>(content.c_str()),
        content.size(), "input");

    xerces::Wrapper4InputSource domInput(&xmlInput, false);

    xerces::DOMDocument *document;
    try {
        document = parser->parse(&domInput);
    } catch (xerces::DOMException const &e) {
        throw Error(std::string("Could not parse XML data: ") + UTF8(e.getMessage()));
    }

    for (std::list<MarkerQuery>::const_iterator iter = queries.begin();
         iter != queries.end(); ++iter)
    {
        AutoRelease<xerces::DOMXPathExpression> expression(0);
        try {
            expression.set(document->createExpression(X(iter->query.c_str()), 0));
        } catch (xerces::DOMXPathException const &) {
            throw Error("Could not parse expression.");
        } catch (xerces::DOMException const &) {
            throw Error("Could not resolve namespace prefixes.");
        }

        AutoRelease<xerces::DOMXPathResult> result(0);
        try {
            result.set(expression->evaluate(document,
                                            xerces::DOMXPathResult::ITERATOR_RESULT_TYPE, 0));
        } catch (xerces::DOMXPathException const &e) {
            throw Error("Could not retrieve an iterator over evaluation results.");
        } catch (xerces::DOMException &e) {
            throw Error("Could not evaluate the expression on the given document.");
        }
        
        std::list<xerces::DOMNode *> markNodes;
        
        while (result->iterateNext())
        {
            xerces::DOMNode *node;
            try {
              node = result->getNodeValue();
            } catch (xerces::DOMXPathException &e) {
              throw Error("Matching node value invalid while marking nodes.");
            }

            // Skip non-element nodes
            if (node->getNodeType() != xerces::DOMNode::ELEMENT_NODE)
                continue;
            
            markNodes.push_back(node);
        }
        
        for (std::list<xerces::DOMNode *>::iterator nodeIter = markNodes.begin();
             nodeIter != markNodes.end(); ++nodeIter)
        {
            xerces::DOMNode *node = *nodeIter;
            
            xerces::DOMNamedNodeMap *map = node->getAttributes();
            if (map == 0)
                continue;
            
            // Create new attribute node.
            xerces::DOMAttr *attr;
            try {
                attr = document->createAttribute(X(iter->attr.c_str()));
            } catch (xerces::DOMException const &e) {
                throw Error("Attribute name contains invalid character.");
            }
            attr->setNodeValue(X(iter->value.c_str()));
            
            map->setNamedItem(attr);
            
        }
    }

    // Serialize DOM tree
    AutoRelease<xerces::DOMLSSerializer> serializer(xqillaImplementation->createLSSerializer());
    AutoRelease<xerces::DOMLSOutput> output(xqillaImplementation->createLSOutput());
    xerces::MemBufFormatTarget target;
    output->setByteStream(&target);
    serializer->write(document, output.get());
    
    std::string outData(reinterpret_cast<char const *>(target.getRawBuffer()),
        target.getLen());
        
    return outData;
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
        throw alpinocorpus::Error(e.what());
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
