/*
 * Oracle Berkeley DB XML-based treebank reader.
 * Written by Lars Buitinck.
 *
 * The basic format is explained in the header. An additional notes:
 *  - We use DBML_WELL_FORMED_ONLY to prevent having to read the DTD.
 *    This only works because the DTD does not define entities.
 */

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

#include <QString>
#include <QUrl>
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

namespace db = DbXml;

namespace xerces = XERCES_CPP_NAMESPACE;

namespace {
    QString toQString(std::string const &s)
    {
        return QString::fromUtf8(s.c_str());
    }
}

namespace alpinocorpus {

class DbCorpusReaderPrivate : public CorpusReader
{
    // XXX mutable is hideous, but saves a lot of const_casts: the read
    // methods are nominally const (don't change future behavior and are
    // thread-safe), but DB XML doesn't expose const reading methods.
    DbXml::XmlManager   mutable mgr;
    DbXml::XmlContainer mutable container;
    std::string collection;
    
    class DbIter : public CorpusReader::IterImpl
    {
    public:
        DbIter(DbXml::XmlContainer &);
        DbIter(DbXml::XmlManager &);
        
        std::string current() const;
        bool equals(IterImpl const &) const;
        void next();
        
    protected:
        mutable DbXml::XmlResults r;
        
        DbIter(DbXml::XmlResults const &);
    };
    
    struct QueryIter : public DbIter
    {
        QueryIter(DbXml::XmlResults const &);
        QString contents(CorpusReader const &) const;
    };
    
public:
    DbCorpusReaderPrivate(QString const &);
    virtual ~DbCorpusReaderPrivate();
    EntryIterator getBegin() const;
    EntryIterator getEnd() const;
    size_t getSize() const
    {
        return const_cast<DbXml::XmlContainer &>(container).getNumDocuments();
    }
    bool validQuery(QueryDialect d, bool variables, QString const &query) const;
    QString readEntry(std::string const &) const;
    QString readEntryMarkQueries(std::string const &entry, QList<MarkerQuery> const &queries) const;
    EntryIterator runXPath(std::string const &) const;
    EntryIterator runXQuery(std::string const &) const;
    
private:
    void setNameAndCollection(QString const &);
    
};
    
DbCorpusReader::DbCorpusReader(QString const &name) :
    d_private(new DbCorpusReaderPrivate(name))
{
}

DbCorpusReader::~DbCorpusReader()
{
}

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
            if (!self.r.hasNext() && !other.r.hasNext())
                return true;        // both at end()
        } catch (db::XmlException const &e) {
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
        throw alpinocorpus::Error(e.what());
    }
}

DbCorpusReaderPrivate::QueryIter::QueryIter(db::XmlResults const &r)
 : DbIter(r)
{
}

QString DbCorpusReaderPrivate::QueryIter::contents(CorpusReader const &) const
{
    db::XmlValue v;
    r.peek(v);
    return toQString(v.getNodeValue());
}
    
CorpusReader::EntryIterator DbCorpusReader::getBegin() const
{
    return d_private->getBegin();
}

CorpusReader::EntryIterator DbCorpusReader::getEnd() const
{
    return d_private->getEnd();
}

size_t DbCorpusReader::getSize() const
{
    return d_private->getSize();
}
    
bool DbCorpusReader::validQuery(QueryDialect d, bool variables, QString const &query) const
{
    return d_private->isValidQuery(d, variables, query);
}


QString DbCorpusReader::readEntry(std::string const &entry) const
{
    return d_private->readEntry(entry);
}
    
QString DbCorpusReader::readEntryMarkQueries(std::string const &entry, 
    QList<MarkerQuery> const &queries) const
{
    return d_private->readEntryMarkQueries(entry, queries);
}

CorpusReader::EntryIterator DbCorpusReader::runXPath(std::string const &query) const
{
    return d_private->runXPath(query);
}

CorpusReader::EntryIterator DbCorpusReader::runXQuery(std::string const &query) const
{
    return d_private->runXQuery(query);
}
    
DbCorpusReaderPrivate::DbCorpusReaderPrivate(QString const &qpath)
 : mgr(), container()
{
    std::string path(qpath.toUtf8().data());

    try {
        db::XmlContainerConfig config;
        config.setReadOnly(true);
        container = mgr.openContainer(path, config);
        // Nasty: using a hard-coded alias to work use in the xpath queries.
        container.addAlias("corpus"); 
        setNameAndCollection(qpath);
    } catch (db::XmlException const &e) {
        throw OpenError(qpath, QString::fromUtf8(e.what()));
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

bool DbCorpusReaderPrivate::validQuery(QueryDialect d, bool variables, QString const &query) const
{
    try {
        db::XmlQueryContext ctx = mgr.createQueryContext();
        QByteArray queryData(query.toUtf8());
        mgr.prepare(queryData.constData(), ctx);
    } catch (db::XmlException const &e) {
        return false;
    }
    
    return true;
}


    QString DbCorpusReaderPrivate::readEntry(std::string const &filename) const
{
    try {
        db::XmlDocument doc(container.getDocument(filename, db::DBXML_LAZY_DOCS));
        std::string content;
        return toQString(doc.getContent(content));

    } catch (db::XmlException const &e) {
        std::ostringstream msg;
        msg << "entry \""                  << filename
            << "\" cannot be read from \"" << container.getName()
            << "\" ("                      << e.what()
            << ")";
        throw Error(msg.str());
    }
}
    
QString DbCorpusReaderPrivate::readEntryMarkQueries(std::string const &entry,
    QList<MarkerQuery> const &queries) const
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

    for (QList<MarkerQuery>::const_iterator iter = queries.begin();
         iter != queries.end(); ++iter)
    {
        QByteArray utf8Query(iter->query.toUtf8());
        QByteArray attrArray(iter->attr.toUtf8());
        QByteArray valArray(iter->value.toUtf8());

        AutoRelease<xerces::DOMXPathExpression> expression(0);
        try {
            expression.set(document->createExpression(X(utf8Query.constData()), 0));
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
        
        QList<xerces::DOMNode *> markNodes;
        
        while (result->iterateNext())
        {
            xerces::DOMNode *node = result->getNodeValue();
            
            // Skip non-element nodes
            if (node->getNodeType() != xerces::DOMNode::ELEMENT_NODE)
                continue;
            
            markNodes.append(node);
        }
        
        for (QList<xerces::DOMNode *>::iterator iter = markNodes.begin();
             iter != markNodes.end(); ++iter)
        {
            xerces::DOMNode *node = *iter;
            
            xerces::DOMNamedNodeMap *map = node->getAttributes();
            if (map == 0)
                continue;
            
            // Create new attribute node.
            xerces::DOMAttr *attr;
            try {
                attr = document->createAttribute(X(attrArray.constData()));
            } catch (xerces::DOMException const &e) {
                throw Error("Attribute name contains invalid character.");
            }
            attr->setNodeValue(X(valArray.constData()));
            
            map->setNamedItem(attr);
            
        }
    }

    // Serialize DOM tree
    AutoRelease<xerces::DOMLSSerializer> serializer(xqillaImplementation->createLSSerializer());
    AutoRelease<xerces::DOMLSOutput> output(xqillaImplementation->createLSOutput());
    xerces::MemBufFormatTarget target;
    output->setByteStream(&target);
    serializer->write(document, output.get());
    
    QByteArray outArray(reinterpret_cast<char const *>(target.getRawBuffer()),
        target.getLen());
        
    return QString::fromUtf8(outArray);
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
        return EntryIterator(new QueryIter(r));
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
void DbCorpusReaderPrivate::setNameAndCollection(QString const &path)
{
    //collection = QFileInfo(path).absoluteFilePath().toLocal8Bit().data();

    setName(toQString(container.getName()));

	QString uri = QString("/%1").arg(name());
	collection = std::string(QUrl::toPercentEncoding(uri));
}

}   // namespace alpinocorpus
