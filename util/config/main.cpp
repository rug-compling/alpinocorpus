#include <AlpinoCorpus/Config.hh>
#include <iostream>
#include <cstring>

using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    alpinocorpus::Config cfg;

    if (argc != 2) {
        cout << "Alpino Corpus " << cfg.Version() << " (" << cfg.Release() << ")" << endl
             << "Usage: " << argv[0] << " OPTION" << endl
             << "Known values for OPTION are:" << endl
             << endl
             << "  --version              output version information" << endl
             << "  --version-major        output major version number" << endl
             << "  --version-minor        output minor version number" << endl
             << "  --version-revision     output revision version number" << endl
             << "  --release              output release info" << endl
             << "  --options              output list of enabled options" << endl;
        return 0;
    }

    if (! strcmp (argv [1], "--version"))
        cout << cfg.Version() << endl;
    else if (! strcmp (argv [1], "--version-major"))
        cout << cfg.VersionMajor() << endl;
    else if (! strcmp (argv [1], "--version-minor"))
        cout << cfg.VersionMinor() << endl;
    else if (! strcmp (argv [1], "--version-revision"))
        cout << cfg.VersionRevision() << endl;
    else if (! strcmp (argv [1], "--release"))
        cout << cfg.Release() << endl;
    else if (! strcmp (argv [1], "--options"))
        cout << cfg.Options() << endl;
    else
        cout << "Invalid argument: " << argv[1] << endl;
}
