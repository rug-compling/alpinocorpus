#include "textfile.ih"

QString alpinocorpus::readFile(QString const &filename)
{
    QFileInfo p(filename);
	if (!p.isFile())
    {
        QByteArray filenameData(filename.toUtf8());
        throw runtime_error(string("readFile: '") + filenameData.constData() +
                            "' is not a regular file!");
    }

    QFile dataFile(filename);
    dataFile.open(QFile::ReadOnly);
    QTextStream dataStream(&dataFile);

    return dataStream.readAll();
}
