#include <cmath>
#include <string>

#include <AlpinoCorpus/IterImpl.hh>

#ifdef _MSC_VER
#include <float.h>
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
#endif

namespace alpinocorpus {
    bool IterImpl::hasProgress()
    {
        return false;
    }

    double IterImpl::progress()
    {
        return NAN;
    }

    void IterImpl::interrupt()
    {
        // XXX no default behavior implemented
    }
}
