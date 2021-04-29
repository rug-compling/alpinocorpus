#ifndef ALPINOCORPUS_XSLTTRANSFORMER_HH
#define ALPINOCORPUS_XSLTTRANSFORMER_HH

#include <string>

#include <memory>

extern "C" {
#include <libxslt/xsltInternals.h>
}

namespace alpinocorpus {

class Stylesheet
{
public:
    Stylesheet(std::string const &xslt);
    ~Stylesheet();
    std::string transform(std::string const &xml) const;
private:
    Stylesheet(Stylesheet const &other);
    Stylesheet &operator=(Stylesheet const &other);
    void initWithStylesheet(std::string const &xslt);

    std::shared_ptr<xsltStylesheet> d_xslPtr;
};

}

#endif // ALPINOCORPUS_XSLTTRANSFORMER_HH
