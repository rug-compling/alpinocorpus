#ifndef ALPINO_CORPUSWRITER_BASE64
#define ALPINO_CORPUSWRITER_BASE64

#include <QByteArray>

QByteArray b64_encode(unsigned long val);
unsigned long b64_decode(QByteArray const &val);

#endif // ALPINO_CORPUSWRITER_BASE64
