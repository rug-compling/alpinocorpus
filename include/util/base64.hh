#ifndef ALPINO_CORPUSWRITER_BASE64
#define ALPINO_CORPUSWRITER_BASE64

#include <QByteArray>

namespace alpinocorpus { namespace util {

QByteArray b64_encode(unsigned long val);
unsigned long b64_decode(QByteArray const &val);

} } // namespace alpinocorpus::util

#endif // ALPINO_CORPUSWRITER_BASE64
