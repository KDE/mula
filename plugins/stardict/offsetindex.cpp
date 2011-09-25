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

#include "offsetindex.h"
#include "file.h"

#include <QtCore/QVector>
#include <QtCore/QFile>
#include <QtCore/QtGlobal>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtGui/QDesktopServices>

#include <arpa/inet.h>

using namespace MulaPluginStarDict;

class OffsetIndex::Private
{
    public:
        Private()
            : wordCount(0)
            , cacheMagicString("StarDict's Cache, Version: 0.1")
            , mappedData(0)
        {   
        }

        ~Private()
        {
        }

        static const int entriesPerPage = 32;
        QString cacheMagic;

        QVector<quint32> wordOffset;
        QFile indexFile;
        ulong wordCount;

        QByteArray wordEntryBuffer; // 256 + sizeof(quint32)*2) - The length of "word_str" should be less than 256. See src/tools/DICTFILE_FORMAT.
        struct indexEntry
        {
            long index;
            QByteArray keyData;

            void assign(ulong i, QByteArray data)
            {
                index = i;
                keyData = data;
            }
        };

        indexEntry first;
        indexEntry last;
        indexEntry middle;
        indexEntry realLast;

        struct pageEntry
        {
            QByteArray keyData;
            quint32 offset;
            quint32 size;
        };

        QByteArray pageData;
        struct page_t
        {
            page_t()
                :index(-1)
            {
            }

            void fill(QByteArray data, int nent, long index_)
            {
                index = index_;
                ulong position = 0;
                for (int i = 0; i < nent; ++i) 
                {    
                    entries[i].keyData = data.mid(position); 
                    position = qstrlen(data.mid(position)) + 1;
                    entries[i].offset = ntohl(*reinterpret_cast<quint32 *>(data.mid(position).data()));
                    position += sizeof(quint32);
                    entries[i].size = ntohl(*reinterpret_cast<quint32 *>(data.mid(position).data()));
                    position += sizeof(quint32);
                }
            }

            long index;
            pageEntry entries[entriesPerPage];
        } page;

        QByteArray cacheMagicString;
        QFile mapFile;
        uchar *mappedData;
};

OffsetIndex::OffsetIndex()
    : d(new Private)
{
}

OffsetIndex::~OffsetIndex()
{
}

QByteArray
OffsetIndex::readFirstOnPageKey(long pageIndex)
{
    d->indexFile.seek(d->wordOffset.at(pageIndex));
    int pageSize = d->wordOffset.at(pageIndex + 1) - d->wordOffset.at(pageIndex);
    d->wordEntryBuffer = d->indexFile.read(qMin(d->wordEntryBuffer.size(), pageSize)); //TODO: check returned values, deal with word entry that strlen>255.
    return d->wordEntryBuffer;
}

QByteArray
OffsetIndex::firstOnPageKey(long pageIndex)
{
    if (pageIndex < d->middle.index)
    {
        if (pageIndex == d->first.index)
            return d->first.keyData;

        return readFirstOnPageKey(pageIndex);
    }
    else if (pageIndex > d->middle.index)
    {
        if (pageIndex == d->last.index)
            return d->last.keyData;

        return readFirstOnPageKey(pageIndex);
    }
    else
    {
        return d->middle.keyData;
    }
}

QStringList
OffsetIndex::cacheVariant(const QString& url)
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
OffsetIndex::loadCache(const QString& url)
{
    QStringList urlStrings = cacheVariant(url);

    foreach (const QString& urlString, urlStrings)
    {
        QFileInfo fileInfoIndex(url);
        QFileInfo fileInfoCache(urlString);

        if (fileInfoCache.lastModified() < fileInfoIndex.lastModified())
            continue;

        d->mapFile.unmap(d->mappedData);
        d->mapFile.setFileName(urlString);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << urlString;
            return -1;
        }

        d->mappedData = d->mapFile.map(0, d->mapFile.size());
        if (d->mappedData == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(urlString);
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
OffsetIndex::saveCache(const QString& url)
{
    QStringList urlStrings = cacheVariant(url);
    foreach (const QString& urlString, urlStrings)
    {
        QFile file(urlString);
        if( !file.open( QIODevice::WriteOnly ) )
        {
            qDebug() << "Failed to open file for writing:" << urlString;
            return -1;
        }

        d->mapFile.setFileName(urlString);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << urlString;
            return -1;
        }

        d->mappedData = d->mapFile.map(0, d->mapFile.size());
        if (d->mappedData == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(urlString);
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
OffsetIndex::load(const QString& url, long wc, qulonglong fileSize)
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

            position += strlen(byteArray.mid(position)) + 1 + 2 * sizeof(quint32);
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

    d->first.assign(0, readFirstOnPageKey(0));
    d->last.assign(d->wordOffset.size() - 2, readFirstOnPageKey(d->wordOffset.size() - 2));
    d->middle.assign((d->wordOffset.size() - 2) / 2, readFirstOnPageKey((d->wordOffset.size() - 2) / 2));
    d->realLast.assign(wc - 1, key(wc - 1));

    return true;
}

ulong
OffsetIndex::loadPage(long pageIndex)
{
    ulong entryCount = d->entriesPerPage;
    if (pageIndex == ulong(d->wordOffset.size() - 2))
        if ((entryCount = d->wordCount % d->entriesPerPage) == 0)
            entryCount = d->entriesPerPage;


    if (pageIndex != d->page.index)
    {
        d->indexFile.seek(d->wordOffset.at(pageIndex));

        d->pageData = d->indexFile.read(d->wordOffset[pageIndex + 1] - d->wordOffset[pageIndex]);
        d->page.fill(d->pageData, entryCount, pageIndex);
    }

    return entryCount;
}

QByteArray
OffsetIndex::key(long index)
{
    loadPage(index / d->entriesPerPage);
    ulong indexInPage = index % d->entriesPerPage;
    setWordEntryOffset(d->page.entries[indexInPage].offset);
    setWordEntrySize(d->page.entries[indexInPage].size);

    return d->page.entries[indexInPage].keyData;
}

void
OffsetIndex::data(long index)
{
    key(index);
}

QByteArray
OffsetIndex::keyAndData(long index)
{
    return key(index);
}

bool
OffsetIndex::lookup(const QByteArray& word, long &index)
{
    bool found = false;
    long indexFrom;
    long indexTo = d->wordOffset.size() - 2;
    int cmpint;
    long indexThisIndex;
    if (stardictStringCompare(word, d->first.keyData) < 0)
    {
        index = 0;
        return false;
    }
    else if (stardictStringCompare(word, d->realLast.keyData) > 0)
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
            cmpint = stardictStringCompare(word, d->page.entries[indexThisIndex].keyData);
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
OffsetIndex::wordEntryOffset() const
{
    return IndexFile::wordEntryOffset();
}

void
OffsetIndex::setWordEntryOffset(quint32 wordEntryOffset)
{
    IndexFile::setWordEntryOffset(wordEntryOffset);
}

quint32
OffsetIndex::wordEntrySize() const
{
    return IndexFile::wordEntrySize();
}

void
OffsetIndex::setWordEntrySize(quint32 wordEntrySize)
{
    IndexFile::setWordEntrySize(wordEntrySize);
}
