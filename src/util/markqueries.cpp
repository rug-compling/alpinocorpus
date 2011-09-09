#include <list>
#include <string>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

#include <xqilla/xqilla-dom3.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/Error.hh>

namespace xerces = XERCES_CPP_NAMESPACE;

namespace alpinocorpus {

std::string markQueries(std::string data,
  std::list<CorpusReader::MarkerQuery> const &queries)
{
    // Prepare the DOM parser.
    xerces::DOMImplementation *xqillaImplementation =
        xerces::DOMImplementationRegistry::getDOMImplementation(X("XPath2 3.0"));
    AutoRelease<xerces::DOMLSParser> parser(xqillaImplementation->createLSParser(
        xerces::DOMImplementationLS::MODE_SYNCHRONOUS, 0));
    
    // Parse the document.
    xerces::MemBufInputSource xmlInput(reinterpret_cast<XMLByte const *>(data.c_str()),
        data.size(), "input");

    xerces::Wrapper4InputSource domInput(&xmlInput, false);

    xerces::DOMDocument *document;
    try {
        document = parser->parse(&domInput);
    } catch (xerces::DOMException const &e) {
        throw Error(std::string("Could not parse XML data: ") + UTF8(e.getMessage()));
    }

    for (std::list<CorpusReader::MarkerQuery>::const_iterator iter = queries.begin();
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

}
