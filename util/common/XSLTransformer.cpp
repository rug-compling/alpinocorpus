#include <stdexcept>
#include <string>

#include "XSLTransformer.hh"

extern "C" {
#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
}

XSLTransformer::XSLTransformer(std::string const &stylesheet)
{
  xmlDocPtr xslDoc = xmlReadMemory(stylesheet.c_str(),
	stylesheet.size(), 0, 0, 0);
  d_stylesheet = xsltParseStylesheetDoc(xslDoc);
}

XSLTransformer::~XSLTransformer()
{
    xsltFreeStylesheet(d_stylesheet);
}

std::string XSLTransformer::transform(std::string const &data)
{
  // Read XML data intro an xmlDoc.
  xmlDocPtr doc = xmlReadMemory(data.c_str(), data.size(), 0, 0, 0);

  if (!doc)
    throw std::runtime_error("XSLTransformer::transform: Could not parse XML data");

    /*
    // Hmpf, data conversions.
    char const **cParams = new char const *[params.size() * 2 + 1];
    int i = 0;
    for (QHash<QString, QString>::const_iterator iter = params.constBegin();
        iter != params.constEnd(); ++iter)
    {
        QByteArray keyData(iter.key().toUtf8());
        QByteArray valueData(iter.value().toUtf8());

        char const *cKey = strdup(keyData.constData());
        char const *cValue = strdup(valueData.constData());

        cParams[i] = cKey;
        cParams[i + 1] = cValue;

            i += 2;
    }

    cParams[params.size() * 2] = 0; // Terminator
    */

  xsltTransformContextPtr ctx = xsltNewTransformContext(d_stylesheet, doc);

  // Transform...
  xmlDocPtr res = xsltApplyStylesheetUser(d_stylesheet, doc, NULL, NULL,
    NULL, ctx);

  if (!res)
  {
    xsltFreeTransformContext(ctx);
    xmlFreeDoc(doc);
    throw std::runtime_error("XSLTransformer::transform: Could not apply transformation!");
  }

  else if (ctx->state != XSLT_STATE_OK)
  {
    xsltFreeTransformContext(ctx);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);
    throw std::runtime_error("XSLTransformer::transform: Transformation error, check your query!");
  }

  xsltFreeTransformContext(ctx);

  xmlChar *output = 0;
  int outputLen = -1;
  xsltSaveResultToString(&output, &outputLen, res, d_stylesheet);

  if (!output)
  {
    xmlFreeDoc(res);
    xmlFreeDoc(doc);
    throw std::runtime_error("Could not apply stylesheet!");
  }

  std::string result(reinterpret_cast<char const *>(output));

/*
  // Deallocate parameter memory
  for (int i = 0; i < params.size() * 2; ++i)
    free(const_cast<char *>(cParams[i]));
  delete[] cParams;
*/

  // Deallocate memory used for libxml2/libxslt.
  xmlFree(output);
  xmlFreeDoc(res);
  xmlFreeDoc(doc);

  return result;
}