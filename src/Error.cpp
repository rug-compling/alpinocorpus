#include <AlpinoCorpus/Error.hh>

#include <QtGlobal>
#include <sstream>

namespace alpinocorpus {
    std::string DuplicateKey::construct(QString const &key)
    {
        return std::string("duplicate key: ") + qPrintable(key);
    }

    /*
     * Construct error message from path and optional extra information
     */
    std::string OpenError::construct(QString const &path)
    {
        return construct(path, "");
    }

    std::string OpenError::construct(QString const &path, QString const &extra)
    {
        return construct(path, qPrintable(data));
    }

    std::string OpenError::construct(QString const &path, char const *extra)
    {
        std::ostringstream msg;
        msg << "Cannot open \"" << qPrintable(path) << "\"";
        if (*extra)
            msg << ": " << extra;
        return msg.str();
    }
}
