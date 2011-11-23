#ifndef ALPINOCORPUS_PUBLIC_CONFIG_HH
#define ALPINOCORPUS_PUBLIC_CONFIG_HH



namespace alpinocorpus {

/*! \class Config Config.hh "AlpinoCorpus/GetUrl.hh"
 *  \brief Config is a class with information about the current build of AlpinoCorpus.
 */

    class Config {

    public:

        Config();
        ~Config();

        //! Get the version string.
        char const * Version() const;
        //! Get the major version number.
        int VersionMajor() const;
        //! Get the minor version number.
        int VersionMinor() const;

        //! Was the library built with Berkeley DB XML?
        bool WithDBXML() const;

        //! Get list op enabled options
        char const * Options() const;

    private:

        int d_major;
        int d_minor;

    };



} // namespace alpinocorpus


#endif // ALPINOCORPUS_PUBLIC_CONFIG_HH
