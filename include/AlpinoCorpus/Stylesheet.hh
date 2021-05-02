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
    virtual ~Stylesheet();

    Stylesheet(std::shared_ptr<xsltStylesheet> xslPtr) : d_xslPtr(xslPtr) {}

    /**
     * Read a stylesheet from an in-memory buffer.
     * @param data Buffer
     * @return Stylesheet
     */
    static Stylesheet *readData(std::string const &data);

    /**
     * Read a stylesheet from a file.
     * @param filename Name of the file containing the stylesheet
     * @return Stylesheet
     */
    static Stylesheet *readFile(std::string const &filename);

    std::string transform(std::string const &xml) const;
private:
    std::shared_ptr<xsltStylesheet> d_xslPtr;
};

}

#endif // ALPINOCORPUS_XSLTTRANSFORMER_HH
