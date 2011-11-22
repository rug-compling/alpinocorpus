#include <config.hh>
#include <AlpinoCorpus/Config.hh>
#include <cstdio>

namespace alpinocorpus {

    Config::Config()
    {
        int i = sscanf(ALPINOCORPUS_VERSION, "%i.%i", &d_major, &d_minor);
        if (i < 2)
            d_minor = -1;
        if (i < 1)
            d_major = -1;
    }

    Config::~Config()
    {
    }

    char const * Config::Version() const
    {
        return ALPINOCORPUS_VERSION;
    }

    int Config::VersionMajor() const
    {
        return d_major;
    }

    int Config::VersionMinor() const
    {
        return d_minor;
    }

    bool Config::WithDBXML() const
    {
#ifdef ALPINOCORPUS_WITH_DBXML
        return true;
#else
        return false;
#endif
    }


} // namespace alpinocorpus

