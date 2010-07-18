#ifndef CORPUSWRITER_BASE64
#define CORPUSWRITER_BASE64

#include <string>

std::string b64_encode(unsigned long val);
unsigned long b64_decode(std::string const &val);

#endif // CORPUSWRITER_BASE64
