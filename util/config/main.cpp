/*! \todo command line options:
 *  - --prefix
 *  - --version
 *  - --ldflags
 *  - --cflags
 * etc.
 */

#include <AlpinoCorpus/Config.hh>
#include <iostream>

using std::cout;
using std::endl;

int main()
{
    alpinocorpus::Config cfg;

    cout << endl

         << "Alpino Corpus version " << cfg.Version() << endl
         << "version major number :  " << cfg.VersionMajor() << endl
         << "version minor number :  " << cfg.VersionMinor() << endl
         << endl

         << "with Berkeley DB XML :  " << (cfg.WithDBXML() ? "yes" : "no") << endl
         << endl;
}
