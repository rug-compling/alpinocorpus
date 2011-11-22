/*! \todo command line options:
 *  - --prefix
 *  - --version
 *  - --ldflags
 *  - --cflags
 * etc.
 */

#include <config.hh>
#include <iostream>

using std::cout;
using std::endl;

int main()
{
    cout << endl

         <<"Alpino Corpus version " ALPINOCORPUS_VERSION << endl
         << endl

         << "with Berkeley DB XML :  "
#ifdef ALPINOCORPUS_WITH_DBXML
        "yes"
#else
        "no"
#endif
         << endl

         << endl;
}
