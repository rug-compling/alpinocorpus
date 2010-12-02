#ifndef ALPINO_NULLSTREAM_HH
#define ALPINO_NULLSTREAM_HH

#include <iostream>

namespace alpinocorpus {

struct NullStream : std::ostream
{
	NullStream() : std::ios(0), std::ostream(0) {}
};

}

#endif // ALPINO_NULLSTREAM_HH
