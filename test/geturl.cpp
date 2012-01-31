#include <../src/util/GetUrl.hh>
#include <iostream>
#include <fstream>
#include <string>

namespace ac = alpinocorpus;

int main(int argc, char* argv[])
{
    if (argc != 2 and argc != 3) {
        std::cerr << "Usage: " << argv[0] << " url [body]" << std::endl;
        return 1;
    }

    std::string body = "";
    if (argc == 3) {
        std::string line;
        std::ifstream myfile(argv[2]);
        if (myfile.is_open()) {
            while (myfile.good()) {
                std::getline(myfile, line);
                body += line;
                body += "\n";
            }
            myfile.close();
        }
    }

    // std::cout << body;

    try {
        ac::util::GetUrl p(argv [1], body);
        std::cout << "Content-type: \"" << p.content_type() << "\"" << std::endl
                  << "Charset:      \"" << p.charset() << "\"" << std::endl;
        
        //std::cout << "Line 10: " << p.line(9) << std::endl;

        for (std::string line = p.line(0); ! p.eof(); line = p.line())
            std::cout << line << std::endl;
        std::cout << "END" << std::endl;
    }  catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
