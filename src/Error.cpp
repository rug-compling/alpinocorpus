#include <AlpinoCorpus/Error.hh>

#include <string>
#include <sstream>

#include <QString>

namespace alpinocorpus {
    std::string DuplicateKey::construct(std::string const &key)
    {
        return std::string("duplicate key: ") + key;
    }

    std::string NotImplemented::construct(char const *type, char const *func)
    {
        std::ostringstream msg;
        msg << func << " not implemented in class " << type;
        return msg.str();
    }

    /*
     * Construct error message from path and optional extra information
     */
    std::string OpenError::construct(std::string const &path)
    {
        return construct(path, "");
    }

    std::string OpenError::construct(std::string const &path, QString const &extra)
    {
        return construct(path, qPrintable(extra));
    }

    std::string OpenError::construct(std::string const &path, char const *extra)
    {
        std::ostringstream msg;
        msg << "Cannot open \"" << path << "\"";
        if (*extra)
            msg << ": " << extra;
        return msg.str();
    }
}
