#ifndef DZ_ISTREAM_HH
#define DZ_ISTREAM_HH

#include <iostream>
#include <string>

#include <QSharedPointer>

#include "DzIstreamBuf.hh"

namespace indexedcorpus {

class DzIstream : public std::istream
{
public:
	DzIstream(char const *filename); // Let's stick to the standards... :/
	virtual ~DzIstream() {}
private:
	QSharedPointer<DzIstreamBuf> d_streamBuf;
};

}

#endif // DZ_ISTREAM_HH
