#ifndef ALPINOCORPUS_XSLTTRANSFORMER_HH
#define ALPINOCORPUS_XSLTTRANSFORMER_HH

#include <string>

extern "C" {
typedef struct _xsltStylesheet xsltStylesheet;
}

class XSLTransformer
{
public:
  XSLTransformer(std::string const &stylesheet);
  virtual ~XSLTransformer();
  std::string transform(std::string const &data);
private:
  XSLTransformer(XSLTransformer const &other);
  XSLTransformer &operator=(XSLTransformer const &other);

  xsltStylesheet *d_stylesheet;	
};

#endif // ALPINOCORPUS_XSLTTRANSFORMER_HH