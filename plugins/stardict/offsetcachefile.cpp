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

#include "offsetcachefile.h"

#include "file.h"
#include "wordentry.h"

#include <QtCore/QVector>
#include <QtCore/QFile>
#include <QtCore/QtGlobal>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QPair>
#include <QtCore/QtEndian>
#include <QtCore/QStandardPaths>

using namespace MulaPluginStarDict;

class OffsetCacheFile::Private
{
    public:
        Private()
            : wordCount(0)
            , pageIndex(-1)
            , cacheMagicString("StarDict's Cache, Version: 0.1")
            , mappedData(0)
        {
        }

        ~Private()
        {
        }

        // The length of "word_str" should be less than 256, and then offset, size
        static const int wordEntrySize = 256 + sizeof(quint32)*2;

        static const int pageEntryNumber = 32;

        QVector<quint32> pageOffsetList;
        QFile indexFile;
        int wordCount;

        // index/date based key and value pair
        QPair<int, QByteArray> first;
        QPair<int, QByteArray> last;
        QPair<int, QByteArray> middle;
        QPair<int, QByteArray> realLast;

        long pageIndex;
        QList<WordEntry> wordEntryList;

        QByteArray cacheMagicString;
        QFile mapFile;
        uchar *mappedData;
};

OffsetCacheFile::OffsetCacheFile()
    : d(new Private)
{
}

OffsetCacheFile::~OffsetCacheFile()
{
}

QByteArray
OffsetCacheFile::readFirstWordDataOnPage(long pageIndex)
{
    int pageSize = d->pageOffsetList.at(pageIndex + 1) - d->pageOffsetList.at(pageIndex);
    int wordEntrySize = d->wordEntrySize;

    d->indexFile.seek(d->pageOffsetList.at(pageIndex));
    return d->indexFile.read(qMin(wordEntrySize, pageSize)); //TODO: check returned values, deal with word entry that strlen>255.
}

QByteArray
OffsetCacheFile::firstWordDataOnPage(long pageIndex)
{
    if (pageIndex < d->middle.first)
    {
        if (pageIndex == d->first.first)
            return d->first.second;

        return readFirstWordDataOnPage(pageIndex);
    }
    else if (pageIndex > d->middle.first)
    {
        if (pageIndex == d->last.first)
            return d->last.second;

        return readFirstWordDataOnPage(pageIndex);
    }
    else
    {
        return d->middle.second;
    }
}

QStringList
OffsetCacheFile::cacheLocations(const QString& completeFilePath)
{
    QStringList result;
    result.append(completeFilePath + ".oft");

    QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QFileInfo cacheLocationFileInfo(cacheLocation);
    QDir cacheLocationDir;

    if (!cacheLocationFileInfo.exists() && cacheLocationDir.mkdir(cacheLocation) == false)
        return result;

    if (!cacheLocationFileInfo.isDir())
        return result;

    result.append(cacheLocation + QDir::separator() + "sdcv" + QDir::separator() + QFileInfo(completeFilePath).fileName() + ".oft");
    return result;
}

bool
OffsetCacheFile::loadCache(const QString& completeFilePath)
{
    foreach (const QString& cacheLocation, cacheLocations(completeFilePath))
    {
        QFileInfo fileInfoIndex(completeFilePath);
        QFileInfo fileInfoCache(cacheLocation);

        if (fileInfoCache.lastModified() < fileInfoIndex.lastModified())
            continue;

        d->mapFile.unmap(d->mappedData);
        d->mapFile.setFileName(cacheLocation);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << cacheLocation;
            return false;
        }

        d->mappedData = d->mapFile.map(0, d->mapFile.size());
        if (d->mappedData == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(cacheLocation);
            return false;
        }

        if (d->cacheMagicString != d->mappedData)
            continue;

        memcpy(d->pageOffsetList.data(), d->mappedData + d->cacheMagicString.size(), d->pageOffsetList.size()*sizeof(d->pageOffsetList.at(0)));
        return true;
    }

    return false;
}

bool
OffsetCacheFile::saveCache(const QString& completeFilePath)
{
    foreach (const QString& cacheLocation, cacheLocations(completeFilePath))
    {
        QFile file(cacheLocation);
        if( !file.open( QIODevice::WriteOnly ) )
        {
            qDebug() << "Failed to open file for writing:" << cacheLocation;
            return false;
        }

        d->mapFile.unmap(d->mappedData);
        d->mapFile.setFileName(cacheLocation);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << cacheLocation;
            return false;
        }

        d->mappedData = d->mapFile.map(0, d->mapFile.size());
        if (d->mappedData == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(cacheLocation);
            return false;
        }

        if (file.write(d->cacheMagicString) != d->cacheMagicString.size())
            continue;

        if (file.write(reinterpret_cast<const char*>(d->pageOffsetList.data()), sizeof(d->pageOffsetList.at(0))*d->pageOffsetList.size())
                != d->pageOffsetList.at(0)*d->pageOffsetList.size())
            continue;

        file.close();

        qDebug() << "Save to cache" << completeFilePath;

        return true;
    }

    return false;
}

int
OffsetCacheFile::loadPage(int pageIndex)
{
    int wordEntryCountOnPage;

    if (pageIndex == (d->pageOffsetList.size() - 2) && (d->wordCount % d->pageEntryNumber) != 0)
        wordEntryCountOnPage = d->wordCount % d->pageEntryNumber;
    else
        wordEntryCountOnPage = d->pageEntryNumber;

    if (pageIndex != d->pageIndex)
    {
        d->pageIndex = pageIndex;
        d->indexFile.seek(d->pageOffsetList.at(pageIndex));
        QByteArray pageData = d->indexFile.read(d->pageOffsetList.at(pageIndex + 1) - d->pageOffsetList.at(pageIndex));

        ulong position = 0;
        d->wordEntryList.clear();
        d->wordEntryList.reserve(wordEntryCountOnPage);
        for (int i = 0; i < wordEntryCountOnPage; ++i)
        {
            WordEntry wordEntry;
            wordEntry.setData(pageData.mid(position));
            position = qstrlen(pageData.mid(position)) + 1;
            wordEntry.setDataOffset(qFromBigEndian(*reinterpret_cast<quint32 *>(pageData.mid(position).data())));
            position += sizeof(quint32);
            wordEntry.setDataSize(qFromBigEndian(*reinterpret_cast<quint32 *>(pageData.mid(position).data())));
            position += sizeof(quint32);

            d->wordEntryList.append(wordEntry);
        }
    }

    return wordEntryCountOnPage;
}

QByteArray
OffsetCacheFile::key(long index)
{
    loadPage(index / d->pageEntryNumber);
    ulong indexInPage = index % d->pageEntryNumber;
    setWordEntryOffset(d->wordEntryList.at(indexInPage).dataOffset());
    setWordEntrySize(d->wordEntryList.at(indexInPage).dataSize());

    return d->wordEntryList.at(indexInPage).data();
}

bool
OffsetCacheFile::load(const QString& completeFilePath)
{
    if (!loadCache(completeFilePath))
    { //map file will close after finish of block
        d->mapFile.unmap(d->mappedData);
        d->mapFile.setFileName(completeFilePath);
        if (!d->mapFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "Failed to open file:" << completeFilePath;
            return -1;
        }

        d->mappedData = d->mapFile.map(0, d->mapFile.size());
        if (d->mappedData == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(completeFilePath);
            return false;
        }

        QByteArray byteArray = QByteArray::fromRawData(reinterpret_cast<const char*>(d->mappedData), d->mapFile.size());

        int position = 0;
        d->pageOffsetList.clear();
        int wordTerminatorOffsetSizeLength = 1 + 2 * sizeof(quint32);
        d->wordCount = 0;

        while (d->mapFile.size() < position)
        {
            if (d->wordCount % d->pageEntryNumber == 0)
                d->pageOffsetList.append(position);

            position += qstrlen(byteArray.mid(position)) + wordTerminatorOffsetSizeLength;
            ++d->wordCount;
        }

        d->pageOffsetList.append(position);

        if (!saveCache(completeFilePath))
            qDebug() << "Cache update failed";
    }

    d->indexFile.setFileName(completeFilePath);
    if (!d->indexFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file:" << completeFilePath;
        d->pageOffsetList.clear();
        return false;
    }

    d->first = qMakePair(0, readFirstWordDataOnPage(0));
    d->last = qMakePair(d->pageOffsetList.size() - 2, readFirstWordDataOnPage(d->pageOffsetList.size() - 2));
    d->middle = qMakePair((d->pageOffsetList.size() - 2) / 2, readFirstWordDataOnPage((d->pageOffsetList.size() - 2) / 2));
    d->realLast = qMakePair(d->wordCount - 1, key(d->wordCount - 1));

    return true;
}

int
OffsetCacheFile::lookupPage(const QByteArray& word)
{
    int pageIndex = invalidIndex;

    if (stardictStringCompare(word, d->first.second) < 0)
    {
        return invalidIndex;
    }
    else if (stardictStringCompare(word, d->realLast.second) > 0)
    {
        return invalidIndex;
    }
    else
    {
        int indexTo = d->pageOffsetList.size() - 2;
        int indexFrom = 0;
        int indexThisIndex = 0;
        int cmpint;
        while (indexFrom <= indexTo)
        {
            indexThisIndex = (indexFrom + indexTo) / 2;
            cmpint = stardictStringCompare(word, firstWordDataOnPage(indexThisIndex));
            if (cmpint > 0)
                indexFrom = indexThisIndex + 1;
            else if (cmpint < 0)
                indexTo = indexThisIndex - 1;
            else
            {
                return indexThisIndex;
            }
        }

        pageIndex = indexTo;
    }

    return pageIndex;
}

inline bool
lessThanCompare(const QString string1, const QString string2)
{
    return stardictStringCompare(string1, string2) < 0;
}

int
OffsetCacheFile::lookup(const QByteArray& word)
{
    int index = lookupPage(word);

    if (index == -1)
    {
        QStringList wordList;
        foreach (const WordEntry &wordEntry, d->wordEntryList)
            wordList.append(QString::fromUtf8(wordEntry.data()));

        QStringList::iterator i = qBinaryFind(wordList.begin() + 1, wordList.end() - 1, QString::fromUtf8(word), lessThanCompare);

        index *= d->pageEntryNumber;

        if (i == wordList.end())
        {
            index = -1;
        }
        else
        {
            index = i - wordList.begin();
        }
    }
    else
    {
        index *= d->pageEntryNumber;
    }

    return index;
}
