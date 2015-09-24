#ifndef CORPUS_INFO_HH
#define CORPUS_INFO_HH

#include <map>
#include <set>
#include <string>

namespace alpinocorpus {

class CorpusInfo
{
public:
  CorpusInfo(std::set<std::string> elements,
      std::string const &lexicalElement,
      std::string const &tokenAttribute)
    : d_elements(elements),
      d_lexicalElement(lexicalElement),
      d_tokenAttribute(tokenAttribute) {}

  std::set<std::string> const &elements() const;
  std::string const &lexicalElement() const;
  std::string const &tokenAttribute() const;

private:
  std::set<std::string> d_elements;
  std::string d_lexicalElement;
  std::string d_tokenAttribute;
};

inline std::set<std::string> const &CorpusInfo::elements() const
{
    return d_elements;
}

inline std::string const &CorpusInfo::lexicalElement() const
{
    return d_lexicalElement;
}

inline std::string const &CorpusInfo::tokenAttribute() const
{
    return d_tokenAttribute;
}

// Predefined corpora by their top elements.
extern std::map<std::string, CorpusInfo> const PREDEFINED_CORPORA;

extern CorpusInfo const FALLBACK_CORPUS_INFO;

CorpusInfo predefinedCorpusOrFallback(std::string const &type);

}

#endif
