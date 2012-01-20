#include <sstream>
#include <string>

#include "url.hh"

namespace alpinocorpus {
namespace util {

std::string toHex(unsigned char c)
{
    static char const hex[] = "0123456789abcdef";

    std::string hexStr(1, hex[(c >> 4) & 0xf]);
    hexStr += hex[c & 0xf];
    
    return hexStr;
}
    
std::string toPercentEncoding(std::string const &str)
{
    std::ostringstream oss;
    
    for (std::string::const_iterator iter = str.begin();
        iter != str.end(); ++iter)
    {
        unsigned char c = *iter;
        
        if ((c >= 0x61 && c <= 0x7a) ||
            (c >= 0x41 && c <= 0x5a) ||
            (c >= 0x30 && c <= 0x39) ||
            c == 0x2d ||
            c == 0x2e ||
            c == 0x5f ||
            c == 0x7e)
            oss << c;
        else {
            oss << '%';
            oss << toHex(c);
        }
    }
    
    return oss.str();
}

}
}
