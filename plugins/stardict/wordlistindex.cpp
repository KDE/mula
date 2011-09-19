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

#include "wordlistindex.h"

#include <QtCore/QDebug>

#include <arpa/inet.h>

using namespace MulaPluginStarDict;

class WordListIndex::Private
{
    public:
        Private()
        {   
        }

        ~Private()
        {
        }

        QByteArray indexDataBuffer;
        QStringList wordList;
};

WordListIndex::~WordListIndex()
{
}

bool
WordListIndex::load(const QString& filePath, long wc, qulonglong fileSize)
{
    Qfile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << Q_FUNC_INFO << "Failed to open file:" << filePath;
        return -1;
    }

    d->indexDataBuffer = file.read(fileSize);

    file.close();

    // To avoid the calculation in each iteration
    int calculatedConst = 1 + 2 * sizeof(quint32);

    for (int i = 0, j = 0; i < wc; ++i)
    {
        d->wordList[i] = d->indexDataBuf.at(j);
        j += d->indexDataBuf.at(j) + calculatedConst;
    }

    d->wordList.insert(wc, d->indexDataBuf.at(j));

    return true;
}

QByteArray
WordListIndex::key(long index) const
{
    return d->wordList.at(index);
}

void
WordListIndex::data(long index)
{
    int position = index + qstrlen(wordlist.mid(index)) + sizeof(char);
    wordentry_offset = ntohl(*reinterpret_cast<quint32 *>(d->wordList[position]));
    position += sizeof(guint32);
    d->wordEntrySize = ntohl(*reinterpret_cast<quint32 *>(d->wordList[position]));
}

QByteArray
WordListIndex::keyAndData(long index)
{
    data(index);
    return key(index);
}

bool
WordListIndex::lookup(const QString &string, long &index)
{
    bool found = false;
    long indexTo = d->wordList.size() - 2;

    if (stardictStringCompare(string, key(0)) < 0)
    {
        index = 0;
    }
    else if (stardictStringCompare(string, key(indexTo)) > 0)
    {
        index = invalidIndex;
    }
    else
    {
        long indexThisIndex = 0;
        long indexFrom = 0;
        int cmpint;
        while (indexFrom <= indexTo)
        {
            indexThisIndex = (indexFrom + indexTo) / 2;
            cmpint = stardictStringCompare(string, key(indexThisIndex));
            if (cmpint > 0)
            {
                indexFrom = indexThisIndex + 1;
            }
            else if (cmpint < 0)
            {
                indexTo = indexThisIndex - 1;
            }
            else
            {
                found = true;
                break;
            }
        }

        if (!found)
            index = indexFrom;    //next
        else
            index = indexThisIndex;
    }

    return found;
}

quint32
WordListIndex::wordEntryOffset() const
{
    return IndexFile::wordEntryOffset();
}

quint32
WordListIndex::wordEntrySize() const
{
    return IndexFile::wordEntrySize();
}
