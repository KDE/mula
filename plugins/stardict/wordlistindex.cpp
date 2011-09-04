/******************************************************************************
 * This file is part of the MULA project
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

using namespace MulaPluginStarDict;

class WordListIndex::Private
{
    public:
        Private()
            : indexDataBuf(0)
        {   
        }

        ~Private()
        {
        }

        QByteArray indexDataBuf;
        QStringList wordList;
}

WordListIndex::~WordListIndex()
{
    delete QByteArray();
}

bool
WordListIndex::load(const QString& url, qulonglong wc, qulonglong fsize)
{
    Qfile file(url);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << Q_FUNC << "Failed to open file:" << fileName;
        return -1;
    }

    d->indexDataBuf = file.read(fsize);

    file.close();

    if (len != fsize)
        return false;

    d->wordList.resize(wc + 1);

    // To avoid the calculation in each iteration
    int calculatedConst = 1 + 2 * sizeof(quint32);
    for (int i = 0, j = 0; i < wc; ++i)
    {
        d->wordList[i] = d->indexDataBuf.at(j);
        j += d->indexDataBuf.at(j) + calculatedConst;
    }

    d->wordList[wc] = d->indexDataBuf.at(j);

    return true;
}

const QString
WordListIndex::key(qlong index) const
{
    return d->wordList(index);
}

void
WordListIndex::data(qlong index)
{
    char *p1 = d->wordList[idx] + strlen(wordlist[idx]) + sizeof(char);
    wordentry_offset = g_ntohl(*reinterpret_cast<quint32 *>(p1));
    p1 += sizeof(guint32);
    wordentry_size = g_ntohl(*reinterpret_cast<quint32 *>(p1));
}

const QByteArray
WordListIndex::keyAndData(qlong index)
{
    data(index);
    return key(index);
}

bool
WordListIndex::lookup(const char *str, qlong &index)
{
    bool found = false;
    qlong iTo = d->wordList.size() - 2;

    if (stardictStringCompare(str, key(0)) < 0)
    {
        index = 0;
    }
    else if (stardictStringCompare(str, key(iTo)) > 0)
    {
        index = INVALID_INDEX;
    }
    else
    {
        qlong iThisIndex = 0;
        qlong iFrom = 0;
        qint cmpint;
        while (iFrom <= iTo)
        {
            iThisIndex = (iFrom + iTo) / 2;
            cmpint = stardictStringCompare(str, key(iThisIndex));
            if (cmpint > 0)
            {
                iFrom = iThisIndex + 1;
            }
            else if (cmpint < 0)
            {
                iTo = iThisIndex - 1;
            }
            else
            {
                found = true;
                break;
            }
        }
        if (!found)
            index = iFrom;    //next
        else
            index = iThisIndex;
    }

    return found;
}

