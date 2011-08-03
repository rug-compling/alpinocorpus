#ifndef ALPINO_CORPUSWRITER_BASE64
#define ALPINO_CORPUSWRITER_BASE64

#include <string>

namespace alpinocorpus { namespace util {

std::string b64_encode(unsigned long val);
unsigned long b64_decode(std::string const &val);

} } // namespace alpinocorpus::util

#endif // ALPINO_CORPUSWRITER_BASE64
