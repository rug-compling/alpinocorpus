#ifndef ALPINOCORPUS_XSLTTRANSFORMER_HH
#define ALPINOCORPUS_XSLTTRANSFORMER_HH

#include <string>

#include <AlpinoCorpus/tr1wrap/memory.hh>

extern "C" {
#include <libxslt/xsltInternals.h>
}

namespace alpinocorpus {

class XSLTransformer
{
public:
    XSLTransformer(std::string const &xslt);
    ~XSLTransformer();
    std::string transform(std::string const &xml) const;
private:
    XSLTransformer(XSLTransformer const &other);
    XSLTransformer &operator=(XSLTransformer const &other);
    void initWithStylesheet(std::string const &xslt);

    std::tr1::shared_ptr<xsltStylesheet> d_xslPtr;
};

}

#endif // ALPINOCORPUS_XSLTTRANSFORMER_HH
