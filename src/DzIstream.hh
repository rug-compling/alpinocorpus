#ifndef DZ_ISTREAM_HH
#define DZ_ISTREAM_HH

#include <iostream>
#include <string>

#include <memory>

#include "DzIstreamBuf.hh"

namespace alpinocorpus {

class DzIstream : public std::istream
{
public:
	DzIstream(char const *filename); // Let's stick to the standards... :/
	virtual ~DzIstream() {}
private:
	std::shared_ptr<DzIstreamBuf> d_streamBuf;
};

}

#endif // DZ_ISTREAM_HH
