#include <AlpinoCorpus/Config.hh>
#include <iostream>
#include <cstring>

using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " OPTION" << endl
             << "Known values for OPTION are:" << endl
             << endl
             << "  --version         output version information" << endl
             << "  --versionmajor    output major version number" << endl
             << "  --versionminor    output minor version number" << endl
             << "  --options         output list of enabled options" << endl;
        return 0;
    }

    alpinocorpus::Config cfg;

    if (! strcmp (argv [1], "--version"))
        cout << cfg.Version() << endl;
    else if (! strcmp (argv [1], "--versionmajor"))
        cout << cfg.VersionMajor() << endl;
    else if (! strcmp (argv [1], "--versionminor"))
        cout << cfg.VersionMinor() << endl;
    else if (! strcmp (argv [1], "--options"))
        cout << cfg.Options() << endl;
    else
        cout << "Invalid argument: " << argv[1] << endl;
}
