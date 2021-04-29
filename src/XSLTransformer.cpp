#include <string>

#include <memory>

extern "C" {
#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
}

#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/XSLTransformer.hh>

namespace alpinocorpus {

XSLTransformer::XSLTransformer(std::string const &xsl)
{
    initWithStylesheet(xsl);
}

XSLTransformer::~XSLTransformer()
{
}

void XSLTransformer::initWithStylesheet(std::string const &xsl)
{
    xmlDocPtr xslDoc = xmlReadMemory(xsl.c_str(), xsl.size(), 0, 0,
        XSLT_PARSE_OPTIONS);

    if (xslDoc == 0)
    {
      throw Error("XSLTransformer::initWithStylesheet: Could not read stylesheet as XML data");
    }

    d_xslPtr.reset(xsltParseStylesheetDoc(xslDoc), xsltFreeStylesheet);

    if (!d_xslPtr)
    {
      throw Error("XSLTransformer::initWithStylesheet: Could not read stylesheet as XML data");
    }
}

std::string XSLTransformer::transform(std::string const &xml) const
{
    // Read XML data intro an xmlDoc.
    std::shared_ptr<xmlDoc> doc(
        xmlReadMemory(xml.c_str(), xml.size(), 0, 0, 0),
        xmlFreeDoc);

    if (!doc)
        throw Error("XSLTransformer::transform: Could not open XML data");

    std::shared_ptr<xsltTransformContext> ctx(
        xsltNewTransformContext(d_xslPtr.get(), doc.get()),
        xsltFreeTransformContext);
    xsltSetCtxtParseOptions(ctx.get(), XSLT_PARSE_OPTIONS);

    // Transform...
    std::shared_ptr<xmlDoc> res(
        xsltApplyStylesheetUser(d_xslPtr.get(), doc.get(), NULL, NULL, NULL, ctx.get()),
        xmlFreeDoc);

    if (!res)
        throw Error("XSLTransformer::transform: Could not apply transformation!");
    else if (ctx->state != XSLT_STATE_OK)
        throw Error("XSLTransformer::transform: Transformation error, check your query!");

    xmlChar *bareOutput = 0;
    int outputLen = -1;
    xsltSaveResultToString(&bareOutput, &outputLen, res.get(), d_xslPtr.get());
    std::shared_ptr<xmlChar> output(bareOutput, xmlFree); 

    if (!output)
      return std::string();

    std::string result(reinterpret_cast<char const *>(output.get()));

    return result;
}

}
