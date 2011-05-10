#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextCodec>
#include <QTextStream>

#include <AlpinoCorpus/util/textfile.hh>

namespace alpinocorpus { namespace util {

QString readFile(QString const &filename)
{
    QFileInfo p(filename);
    if (!p.isFile()) {
        QByteArray filenameData(filename.toUtf8());
        throw std::runtime_error(std::string("readFile: '")
                                + filenameData.constData()
                                + "' is not a regular file");
    }

    QFile dataFile(filename);
    dataFile.open(QFile::ReadOnly);
    QTextStream dataStream(&dataFile);
    dataStream.setCodec(QTextCodec::codecForName("UTF-8"));

    return dataStream.readAll();
}

} } // namespace alpinocorpus::util
