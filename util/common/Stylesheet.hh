#ifndef ALPINOCORPUS_XSLTTRANSFORMER_HH
#define ALPINOCORPUS_XSLTTRANSFORMER_HH

#include <string>

extern "C" {
typedef struct _xsltStylesheet xsltStylesheet;
}

class Stylesheet
{
public:
  /**
   * Construct a Stylesheet instance from plain text.
   */
  Stylesheet(std::string const &data);

  virtual ~Stylesheet();

  /**
   * Transform XML data using this stylesheet.
   */
  std::string transform(std::string const &data);
private:
  Stylesheet(Stylesheet const &other);
  Stylesheet &operator=(Stylesheet const &other);

  xsltStylesheet *d_stylesheet;	
};

#endif // ALPINOCORPUS_XSLTTRANSFORMER_HH
