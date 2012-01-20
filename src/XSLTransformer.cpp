#include <string>

extern "C" {
#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
}

#include "AlpinoCorpus/Error.hh"

#include "XSLTransformer.hh"

namespace alpinocorpus {

XSLTransformer::XSLTransformer(std::string const &xsl)
{
    initWithStylesheet(xsl);
}

XSLTransformer::~XSLTransformer()
{
    xsltFreeStylesheet(d_xslPtr);
}

void XSLTransformer::initWithStylesheet(std::string const &xsl)
{
    xmlDocPtr xslDoc = xmlReadMemory(xsl.c_str(), xsl.size(), 0, 0, 0);
    d_xslPtr = xsltParseStylesheetDoc(xslDoc);
}

std::string XSLTransformer::transform(std::string const &xml) const
{
    // Read XML data intro an xmlDoc.
    xmlDocPtr doc = xmlReadMemory(xml.c_str(), xml.size(), 0, 0, 0);

    if (!doc)
        throw Error("XSLTransformer::transform: Could not open XML data");

    xsltTransformContextPtr ctx = xsltNewTransformContext(d_xslPtr, doc);

    // Transform...
    xmlDocPtr res = xsltApplyStylesheetUser(d_xslPtr, doc, NULL, NULL,
        NULL, ctx);

    if (!res)
    {
        xsltFreeTransformContext(ctx);
        xmlFreeDoc(doc);
        throw Error("XSLTransformer::transform: Could not apply transformation!");
    }
    else if (ctx->state != XSLT_STATE_OK)
    {
        xsltFreeTransformContext(ctx);
        xmlFreeDoc(res);
        xmlFreeDoc(doc);
        throw Error("XSLTransformer::transform: Transformation error, check your query!");
    }

    xsltFreeTransformContext(ctx);

    xmlChar *output = 0;
    int outputLen = -1;
    xsltSaveResultToString(&output, &outputLen, res, d_xslPtr);

    if (!output)
    {
        xmlFreeDoc(res);
        xmlFreeDoc(doc);
        throw Error("Could not apply stylesheet!");
    }

    std::string result(reinterpret_cast<char const *>(output));

    // Deallocate memory used for libxml2/libxslt.
    xmlFree(output);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);

    return result;
}

}
