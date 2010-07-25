#ifndef CORPUSWRITER_BASE64
#define CORPUSWRITER_BASE64

#include <QByteArray>

QByteArray b64_encode(unsigned long val);
unsigned long b64_decode(QByteArray const &val);

#endif // CORPUSWRITER_BASE64
