/*
 * Copyright 2010 Daniel de Kok
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <QObject>
#include <QString>

#include <AlpinoCorpus/QDictZipFile.hh>
#include "QDictZipFilePrivate.hh"

namespace alpinocorpus {

    QDictZipFile::QDictZipFile(QString const &filename, QObject *parent)
    : QIODevice(parent), d_private(new QDictZipFilePrivate(filename, 0)) {}
    
    QDictZipFile::~QDictZipFile() {}
    
    bool QDictZipFile::atEnd()
    {
        return d_private->atEnd();
    }
    
    bool QDictZipFile::open(OpenMode mode)
    {
        bool opened = d_private->open(mode);
        if (opened)
            setOpenMode(mode);
        return opened;
    }
    
    bool QDictZipFile::seek(qint64 pos)
    {
        QIODevice::seek(pos);
        return d_private->seek(pos);
    }
    
    qint64 QDictZipFile::readData(char *data, qint64 maxlen)
    {
        return d_private->readData(data, maxlen);
    }
    
    qint64 QDictZipFile::writeData(const char *data, qint64 len)
    {
        return d_private->writeData(data, len);
    }
}
