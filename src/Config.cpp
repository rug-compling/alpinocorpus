#include <config.hh>
#include <AlpinoCorpus/Config.hh>
#include <cstdio>

namespace alpinocorpus {

    Config::Config()
    {
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
        return ALPINOCORPUS_MAJOR;
    }

    int Config::VersionMinor() const
    {
        return ALPINOCORPUS_MINOR;
    }

    int Config::VersionRevision() const
    {
        return ALPINOCORPUS_REVISION;
    }

    char const * Config::Options() const
    {
        return ""
#ifdef ALPINOCORPUS_WITH_DBXML
            "with-dbxml "
#endif
#ifdef ALPINOCORPUS_WITH_SSL
            "with-ssl "
#endif
#ifdef ALPINOCORPUS_WITH_SSL_STRICT
            "with-ssl-strict "
#endif
            ;
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

