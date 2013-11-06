#ifndef DZ_ISTREAM_HH
#define DZ_ISTREAM_HH

#include <iostream>
#include <string>

#include <boost/shared_ptr.hpp>

#include "DzIstreamBuf.hh"

namespace alpinocorpus {

class DzIstream : public std::istream
{
public:
	DzIstream(char const *filename); // Let's stick to the standards... :/
	virtual ~DzIstream() {}
private:
	boost::shared_ptr<DzIstreamBuf> d_streamBuf;
};

}

#endif // DZ_ISTREAM_HH
