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
#include <QtGui/QDesktopServices>

#include <arpa/inet.h>

using namespace MulaPluginStarDict;

class OffsetCacheFile::Private
{
    public:
        Private()
            : wordCount(0)
            , entryIndex(-1)
            , cacheMagicString("StarDict's Cache, Version: 0.1")
            , mappedData(0)
        {
        }

        ~Private()
        {
        }

        void fill(QByteArray data, int entryCount, long index);

        static const int entriesPerPage = 32;

        QVector<quint32> wordOffset;
        QFile indexFile;
        ulong wordCount;

        QByteArray wordEntryBuffer; // 256 + sizeof(quint32)*2) - The length of "word_str" should be less than 256. See src/tools/DICTFILE_FORMAT.

        QPair<int, QByteArray> first;
        QPair<int, QByteArray> last;
        QPair<int, QByteArray> middle;
        QPair<int, QByteArray> realLast;

        QByteArray pageData;

        long entryIndex;
        QList<WordEntry> entries;

        QByteArray cacheMagicString;
        QFile mapFile;
        uchar *mappedData;
};

void
OffsetCacheFile::Private::fill(QByteArray data, int entryCount, long index)
{
    entryIndex = index;
    ulong position = 0;
    for (int i = 0; i < entryCount; ++i)
    {
        entries[i].setData(data.mid(position));
        position = qstrlen(data.mid(position)) + 1;
        entries[i].setDataOffset(ntohl(*reinterpret_cast<quint32 *>(data.mid(position).data())));
        position += sizeof(quint32);
        entries[i].setDataSize(ntohl(*reinterpret_cast<quint32 *>(data.mid(position).data())));
        position += sizeof(quint32);
    }
}

OffsetCacheFile::OffsetCacheFile()
    : d(new Private)
{
}

OffsetCacheFile::~OffsetCacheFile()
{
}

QByteArray
OffsetCacheFile::readFirstOnPageKey(long pageIndex)
{
    d->indexFile.seek(d->wordOffset.at(pageIndex));
    int pageSize = d->wordOffset.at(pageIndex + 1) - d->wordOffset.at(pageIndex);
    d->wordEntryBuffer = d->indexFile.read(qMin(d->wordEntryBuffer.size(), pageSize)); //TODO: check returned values, deal with word entry that strlen>255.
    return d->wordEntryBuffer;
}

QByteArray
OffsetCacheFile::firstOnPageKey(long pageIndex)
{
    if (pageIndex < d->middle.first)
    {
        if (pageIndex == d->first.first)
            return d->first.second;

        return readFirstOnPageKey(pageIndex);
    }
    else if (pageIndex > d->middle.first)
    {
        if (pageIndex == d->last.first)
            return d->last.second;

        return readFirstOnPageKey(pageIndex);
    }
    else
    {
        return d->middle.second;
    }
}

QStringList
OffsetCacheFile::cacheLocations(const QString& url)
{
    QStringList result;
    result.append(url + ".oft");

    QString cacheLocation = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    QFileInfo cacheLocationFileInfo(cacheLocation);
    QDir cacheLocationDir;

    if (!cacheLocationFileInfo.exists() && cacheLocationDir.mkdir(cacheLocation) == false)
        return result;

    if (!cacheLocationFileInfo.isDir())
        return result;

    result.append(cacheLocation + QDir::separator() + "sdcv" + QDir::separator() + QFileInfo(url).fileName() + ".oft");
    return result;
}

bool
OffsetCacheFile::loadCache(const QString& url)
{
    foreach (const QString& cacheLocation, cacheLocations(url))
    {
        QFileInfo fileInfoIndex(url);
        QFileInfo fileInfoCache(cacheLocation);

        if (fileInfoCache.lastModified() < fileInfoIndex.lastModified())
            continue;

        d->mapFile.unmap(d->mappedData);
        d->mapFile.setFileName(cacheLocation);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << cacheLocation;
            return -1;
        }

        d->mappedData = d->mapFile.map(0, d->mapFile.size());
        if (d->mappedData == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(cacheLocation);
            return false;
        }

        if (d->cacheMagicString != d->mappedData)
            continue;

        memcpy(d->wordOffset.data(), d->mappedData + d->cacheMagicString.size(), d->wordOffset.size()*sizeof(d->wordOffset.at(0)));
        return true;
    }

    return false;
}

bool
OffsetCacheFile::saveCache(const QString& url)
{
    foreach (const QString& cacheLocation, cacheLocations(url))
    {
        QFile file(cacheLocation);
        if( !file.open( QIODevice::WriteOnly ) )
        {
            qDebug() << "Failed to open file for writing:" << cacheLocation;
            return -1;
        }

        d->mapFile.unmap(d->mappedData);
        d->mapFile.setFileName(cacheLocation);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << cacheLocation;
            return -1;
        }

        d->mappedData = d->mapFile.map(0, d->mapFile.size());
        if (d->mappedData == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(cacheLocation);
            return false;
        }

        if (file.write(d->cacheMagicString) != d->cacheMagicString.size())
            continue;

        if (file.write(reinterpret_cast<const char*>(d->wordOffset.data()), sizeof(d->wordOffset.at(0))*d->wordOffset.size())
                != d->wordOffset.at(0)*d->wordOffset.size())
            continue;

        file.close();

        qDebug() << "Save to cache" << url;

        return true;
    }

    return false;
}

bool
OffsetCacheFile::load(const QString& url, int wc, qulonglong fileSize)
{
    Q_UNUSED(fileSize);

    d->wordCount = wc;
    qulonglong npages = (wc - 1) / d->entriesPerPage + 2;
    d->wordOffset.resize(npages);

    if (!loadCache(url))
    { //map file will close after finish of block
        d->mapFile.setFileName(url);
        if (!d->mapFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "Failed to open file:" << url;
            return -1;
        }

        d->mappedData = d->mapFile.map(0, d->mapFile.size());
        if (d->mappedData == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(url);
            return false;
        }

        QByteArray byteArray = QByteArray::fromRawData(reinterpret_cast<const char*>(d->mappedData), d->mapFile.size());

        int position = 0;
        int j = 0;
        for (int i = 0; i < wc; i++)
        {
            if (i % d->entriesPerPage == 0)
            {
                d->wordOffset[j] = position;
                ++j;
            }

            position += qstrlen(byteArray.mid(position)) + 1 + 2 * sizeof(quint32);
        }

        d->wordOffset[j] = position;

        if (!saveCache(url))
            qDebug() << "Cache update failed";
    }

    d->indexFile.setFileName(url);
    if (!d->indexFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file:" << url;
        d->wordOffset.resize(0);
        return false;
    }

    d->first = qMakePair(0, readFirstOnPageKey(0));
    d->last = qMakePair(d->wordOffset.size() - 2, readFirstOnPageKey(d->wordOffset.size() - 2));
    d->middle = qMakePair((d->wordOffset.size() - 2) / 2, readFirstOnPageKey((d->wordOffset.size() - 2) / 2));
    d->realLast = qMakePair(wc - 1, key(wc - 1));

    return true;
}

ulong
OffsetCacheFile::loadPage(long pageIndex)
{
    ulong entryCount = d->entriesPerPage;
    if (pageIndex == ulong(d->wordOffset.size() - 2) && (entryCount = d->wordCount % d->entriesPerPage) == 0)
    {
        entryCount = d->entriesPerPage;
    }

    if (pageIndex != d->entryIndex)
    {
        d->indexFile.seek(d->wordOffset.at(pageIndex));

        d->pageData = d->indexFile.read(d->wordOffset[pageIndex + 1] - d->wordOffset[pageIndex]);
        d->fill(d->pageData, entryCount, pageIndex);
    }

    return entryCount;
}

QByteArray
OffsetCacheFile::key(long index)
{
    loadPage(index / d->entriesPerPage);
    ulong indexInPage = index % d->entriesPerPage;
    setWordEntryOffset(d->entries.at(indexInPage).dataOffset());
    setWordEntrySize(d->entries.at(indexInPage).dataSize());

    return d->entries.at(indexInPage).data();
}

bool
OffsetCacheFile::lookup(const QByteArray& word, long &index)
{
    bool found = false;
    long indexFrom;
    long indexTo = d->wordOffset.size() - 2;
    int cmpint;
    long indexThisIndex;
    if (stardictStringCompare(word, d->first.second) < 0)
    {
        index = 0;
        return false;
    }
    else if (stardictStringCompare(word, d->realLast.second) > 0)
    {
        index = invalidIndex;
        return false;
    }
    else
    {
        indexFrom = 0;
        indexThisIndex = 0;
        while (indexFrom <= indexTo)
        {
            indexThisIndex = (indexFrom + indexTo) / 2;
            cmpint = stardictStringCompare(word, firstOnPageKey(indexThisIndex));
            if (cmpint > 0)
                indexFrom = indexThisIndex + 1;
            else if (cmpint < 0)
                indexTo = indexThisIndex - 1;
            else
            {
                found = true;
                break;
            }
        }
        if (!found)
            index = indexTo;    //prev
        else
            index = indexThisIndex;
    }

    if (!found)
    {
        ulong netr = loadPage(index);
        indexFrom = 1; // Needn't search the first word anymore.
        indexTo = netr - 1;
        indexThisIndex = 0;
        while (indexFrom <= indexTo)
        {
            indexThisIndex = (indexFrom + indexTo) / 2;
            cmpint = stardictStringCompare(word, d->entries.at(indexThisIndex).data());
            if (cmpint > 0)
                indexFrom = indexThisIndex + 1;
            else if (cmpint < 0)
                indexTo = indexThisIndex - 1;
            else
            {
                found = true;
                break;
            }
        }

        index *= d->entriesPerPage;
        if (!found)
            index += indexFrom;    //next
        else
            index += indexThisIndex;
    }
    else
    {
        index *= d->entriesPerPage;
    }

    return found;
}

quint32
OffsetCacheFile::wordEntryOffset() const
{
    return AbstractIndexFile::wordEntryOffset();
}

void
OffsetCacheFile::setWordEntryOffset(quint32 wordEntryOffset)
{
    AbstractIndexFile::setWordEntryOffset(wordEntryOffset);
}

quint32
OffsetCacheFile::wordEntrySize() const
{
    return AbstractIndexFile::wordEntrySize();
}

void
OffsetCacheFile::setWordEntrySize(quint32 wordEntrySize)
{
    AbstractIndexFile::setWordEntrySize(wordEntrySize);
}
