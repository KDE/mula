/******************************************************************************
 * This file is part of the Mula project
 * Copyright (c) 2011 Laszlo Papp <lpapp@kde.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "indexfile.h"

#include "file.h"
#include "wordentry.h"

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QFile>

#include <arpa/inet.h>

using namespace MulaPluginStarDict;

class IndexFile::Private
{
    public:
        Private()
        {   
        }

        ~Private()
        {
        }

        QList<WordEntry> wordEntryList;
};

IndexFile::IndexFile()
    : d(new Private)
{
}

IndexFile::~IndexFile()
{
}

bool
IndexFile::load(const QString& filePath, qulonglong fileSize, int wordCount)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << Q_FUNC_INFO << "Failed to open file:" << filePath;
        return -1;
    }

    QByteArray indexDataBuffer = file.read(fileSize);

    file.close();

    int position = 0;
    d->wordEntryList.clear();
    d->wordEntryList.reserve(wordCount);
    for (int i = 0; i < wordCount; ++i)
    {
        WordEntry wordEntry;
        wordEntry.setData(indexDataBuffer.mid(position));
        ++position;
        wordEntry.setDataOffset(ntohl(*reinterpret_cast<quint32 *>(indexDataBuffer.mid(position).data())));
        position += sizeof(quint32);
        wordEntry.setDataSize(ntohl(*reinterpret_cast<quint32 *>(indexDataBuffer.mid(position).data())));
        position += sizeof(quint32);

        d->wordEntryList.append(wordEntry);
    }

    return true;
}

QByteArray
IndexFile::key(long index)
{
    return d->wordEntryList.at(index).data();
}

inline bool
lessThanCompare(const QString string1, const QString string2)
{
    return stardictStringCompare(string1, string2) < 0;
}

bool
IndexFile::lookup(const QByteArray &word, long &index)
{
    QStringList wordList;
    foreach (const WordEntry &wordEntry, d->wordEntryList)
        wordList.append(QString::fromUtf8(wordEntry.data()));

    QStringList::iterator i = qBinaryFind(wordList.begin(), wordList.end(), QString::fromUtf8(word), lessThanCompare);

    if (i == wordList.end())
        index = -1;
    else
        index = i - wordList.begin();

    return true;
}

quint32
IndexFile::wordEntryOffset() const
{
    return AbstractIndexFile::wordEntryOffset();
}

void
IndexFile::setWordEntryOffset(quint32 offset)
{
    AbstractIndexFile::setWordEntryOffset(offset);
}

quint32
IndexFile::wordEntrySize() const
{
    return AbstractIndexFile::wordEntrySize();
}

void
IndexFile::setWordEntrySize(quint32 size)
{
    AbstractIndexFile::setWordEntrySize(size);
}
