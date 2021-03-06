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
#include <QtCore/QtEndian>

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
IndexFile::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << Q_FUNC_INFO << "Failed to open file:" << filePath;
        return false;
    }

    d->wordEntryList.clear();
    char ch;
    QByteArray data;

    while (!file.atEnd()) {
        WordEntry wordEntry;

        data.clear();

        forever {
            if (file.read(&ch, 1) != -1) {
                if (ch) {
                    data.append(ch);
                } else {
                    break;
                }
            } else {
                return false;
            }
        }

        wordEntry.setData(data);

        wordEntry.setDataOffset(qFromBigEndian(*reinterpret_cast<quint32 *>(file.read(sizeof(quint32)).data())));
        wordEntry.setDataSize(qFromBigEndian(*reinterpret_cast<quint32 *>(file.read(sizeof(quint32)).data())));

        d->wordEntryList.append(wordEntry);
    }

    file.close();

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

int
IndexFile::lookup(const QByteArray &word)
{
    QStringList wordList;
    foreach (const WordEntry &wordEntry, d->wordEntryList)
        wordList.append(QString::fromUtf8(wordEntry.data()));

    QStringList::iterator i = qBinaryFind(wordList.begin(), wordList.end(), QString::fromUtf8(word), lessThanCompare);

    return i == wordList.end() ? -1 : i - wordList.begin();
}
