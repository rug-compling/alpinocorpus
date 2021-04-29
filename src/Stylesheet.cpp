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
#include <AlpinoCorpus/Stylesheet.hh>

namespace {
    alpinocorpus::Stylesheet *readDoc(xmlDocPtr xslDoc) {
        xsltStylesheetPtr xsl = xsltParseStylesheetDoc(xslDoc);
        if (xsl == 0) {
            xmlFreeDoc(xslDoc);
            throw alpinocorpus::Error("Stylesheet::initWithStylesheet: Could not read stylesheet as XML data");
        }

        // The stylesheet has taken ownership of xslDoc, so we do not
        // need to manage it separately.
        std::shared_ptr<xsltStylesheet> xslPtr(xsl, xsltFreeStylesheet);

        return new alpinocorpus::Stylesheet(xslPtr);
    }

}

namespace alpinocorpus {

    Stylesheet *Stylesheet::readData(std::string const &data) {
        xmlDocPtr xslDoc = xmlReadMemory(data.c_str(), data.size(), 0, 0,
                                         XSLT_PARSE_OPTIONS);
        if (xslDoc == 0) {
            throw Error("Stylesheet::initWithStylesheet: Could not read stylesheet as XML data");
        }

        return ::readDoc(xslDoc);
    }

    Stylesheet *Stylesheet::readFile(std::string const &filename) {
        xmlDocPtr xslDoc = xmlReadFile(filename.c_str(), 0, XSLT_PARSE_OPTIONS);
        if (xslDoc == 0) {
            throw Error("Stylesheet::initWithStylesheet: Could not read stylesheet as XML data");
        }

        return ::readDoc(xslDoc);
    }

    Stylesheet::~Stylesheet() {
    }


    std::string Stylesheet::transform(std::string const &xml) const {
        // Read XML data intro an xmlDoc.
        std::shared_ptr<xmlDoc> doc(
                xmlReadMemory(xml.c_str(), xml.size(), 0, 0, 0),
                xmlFreeDoc);

        if (!doc)
            throw Error("Stylesheet::transform: Could not open XML data");

        std::shared_ptr<xsltTransformContext> ctx(
                xsltNewTransformContext(d_xslPtr.get(), doc.get()),
                xsltFreeTransformContext);
        xsltSetCtxtParseOptions(ctx.get(), XSLT_PARSE_OPTIONS);

        // Transform...
        std::shared_ptr<xmlDoc> res(
                xsltApplyStylesheetUser(d_xslPtr.get(), doc.get(), NULL, NULL, NULL, ctx.get()),
                xmlFreeDoc);

        if (!res)
            throw Error("Stylesheet::transform: Could not apply transformation!");
        else if (ctx->state != XSLT_STATE_OK)
            throw Error("Stylesheet::transform: Transformation error, check your query!");

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
