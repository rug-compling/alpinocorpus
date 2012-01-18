#include <util/GetUrl.hh>
#include <iostream>
#include <string>

namespace ac = alpinocorpus;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " url" << std::endl;
        return 1;
    }

    try {
        ac::util::GetUrl p(argv [1]);
        std::cout << "Content-type: \"" << p.content_type() << "\"" << std::endl
                  << "Charset:      \"" << p.charset() << "\"" << std::endl;
        
        for (;;) {
            std::string line = p.line();
            if (line.size())
                std::cout << line;
            else
                break;
        }

        std::cout << "END" << std::endl;
    }  catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
