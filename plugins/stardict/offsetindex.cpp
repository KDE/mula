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
            : entriesPerPage(32)
            , wordCount(0)
            , cacheMagicString("StarDict's Cache, Version: 0.1")
        {   
        }

        ~Private()
        {
        }

        int entriesPerPage;
        QString cacheMagic;

        QVector<quint32> wordOffset;
        QFile indexFile;
        ulong wordCount;

        char wordEntryBuffer[256 + sizeof(quint32)*2]; // The length of "word_str" should be less than 256. See src/tools/DICTFILE_FORMAT.
        struct indexEntry
        {
            ulong index;
            QByteArray keyData;

            void assign(ulong i, QByteArray &data)
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
            QString keyData;
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

            void fill(QByteArray data, int nent, ulong index_)
            {
                index = index_;
                ulong position; 
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

            ulong index;
            pageEntry entries[entriesPerPage];
        } page;

        QByteArray cacheMagicString;
};

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

bool
OffsetIndex::loadCache(const QString& url)
{
    QStringList urlStrings = cacheVariant(url);

    foreach (const QString& urlString, urlStrings)
    {
        QFileInfo fileInfoIndex(url);
        QFileInfo fileInfoCache(url);

        if (fileInfoCache.lastModified() < fileInfoIndex.lastModified())
            continue;

        d->mapFile.setFileName(urlString);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << urlString;
            return -1;
        }

        data = d->mapFile.map(0, fileSize);
        if (data == NULL)
        {
            qDebug << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(urlString);
            return false;
        }

        if (d->cacheMagicString != data)
            continue;

        memcpy(&wordoffset[0], data + d->cacheMagicString.size(), wordoffset.size()*sizeof(wordoffset[0]));
        return true;
    }

    return false;
}

QStringList
OffsetIndex::cacheVariant(const QString& url)
{
    QStringList result;
    result.append(url + ".oft");

    QFileInfo urlFileInfo(url);
    QString cacheLocation = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    QFileInfo cacheLocationFileInfo(cacheLocation);
    QDir cacheLocationDir;

    if (!cacheLocationFileInfo.exists() && cacheLocationDir.mkdir(cacheLocation) == false)
        return result;

    QString cacheDir = cacheLocation + QDir::separator() + "sdcv";

    if (!cacheLocationFileInfo.exists())
    {
        if (!cacheLocationDir.mkdir(cacheLocation))
            return result;
    }
    else if (!cacheLocationFileInfo.isDir())
    {
        return result;
    }

    result.append(cacheDir + QDir::separator() + urlFileInfo.fileName() + ".oft");
    return result;
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

        char *data = d->mapFile.map(0, d->size);
        if (data == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(urlString);
            return false;
        }

        if (file(d->cacheMagicString) != d->cacheMagicString.size())
            continue;

        if (file(d->wordOffset, sizeof(d->wordOffset[0])*d->wordOffset.size()) != d->wordOffset.size())
            continue;

        file.close();

        qDebug() << "Save to cache" << url;

        return true;
    }

    return false;
}

bool
OffsetIndex::load(const QString& url, long wc, long fileSize)
{
    d->wordCount = wc;
    ulonglong npages = (wc - 1) / enterPerPage + 2;
    wordoffset.resize(npages);

    if (!load_cache(url))
    { //map file will close after finish of block
        m_mapFile.setFileName(url);
        if( !m_mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << fileName;
            return -1;
        }

        data = QFile.map(0, m_mapFile.size());
        if (data == NULL)
        {
            qDebug() << Q_FUNC_INFO << QString("Mapping the file %1 failed!").arg(idxfilename);
            return false;
        }

        const gchar *idxdatabuffer = data;

        const gchar *p1 = idxdatabuffer;
        gulong index_size;
        guint32 j = 0;
        for (guint32 i = 0; i < wc; i++)
        {
            index_size = strlen(p1) + 1 + 2 * sizeof(guint32);
            if (i % d->entriesPerPage == 0)
            {
                wordoffset[j] = p1 - idxdatabuffer;
                ++j;
            }
            p1 += index_size;
        }
        wordoffset[j] = p1 - idxdatabuffer;

        if (!saveCache(url))
            qDebug() << "Cache update failed";
    }

    if (!(idxfile = fopen(url.c_str(), "rb")))
    {
        wordoffset.resize(0);
        return false;
    }

    first.assign(0, read_first_on_page_key(0));
    last.assign(wordoffset.size() - 2, read_first_on_page_key(wordoffset.size() - 2));
    middle.assign((wordoffset.size() - 2) / 2, read_first_on_page_key((wordoffset.size() - 2) / 2));
    real_last.assign(wc - 1, get_key(wc - 1));

    return true;
}

ulong
OffsetIndex::loadPage(ulong page_idx)
{
    ulong nentr = d->entriesPerPage;
    if (page_idx == ulong(wordoffset.size() - 2))
        if ((nentr = wordcount % d->entriesPerPage) == 0)
            nentr = d->entriesPerPage;


    if (page_idx != page.idx)
    {
        page_data.resize(wordoffset[pageIndex + 1] - wordoffset[page_idx]);
        fseek(indexFile, wordoffset[pageIndex], SEEK_SET);
        fread(&pageData[0], 1, pageData.size(), indexFile);
        page.fill(&pageData[0], nentr, pageIndex);
    }

    return nentr;
}

QByteArray
OffsetIndex::key(ulong index)
{
    loadPage(idx / d->entriesPerPage);
    ulong indexInPage = idx % d->entriesPerPage;
    wordentryOffset = page.entries[idx_in_page].off;
    wordentrySize = page.entries[idx_in_page].size;

    return page.entries[indexInPage].keystr;
}

void
OffsetIndex::data(ulong index)
{
   key(index);
}

QByteArray
OffsetIndex::keyAndData(ulong index)
{
    return key(index);
}

bool
OffsetIndex::lookup(const char *str, ulong &idx)
{
    bool found = false;
    ulong iFrom;
    ulong iTo = d->wordOffset.size() - 2;
    int cmpint;
    ulong iThisIndex;
    if (stardictStringCompare(str, first.keystr.c_str()) < 0)
    {
        idx = 0;
        return false;
    }
    else if (stardictStringCompare(string, real_last.keystr.c_str()) > 0)
    {
        index = INVALID_INDEX;
        return false;
    }
    else
    {
        iFrom = 0;
        iThisIndex = 0;
        while (iFrom <= iTo)
        {
            iThisIndex = (iFrom + iTo) / 2;
            cmpint = stardict_strcmp(str, get_first_on_page_key(iThisIndex));
            if (cmpint > 0)
                iFrom = iThisIndex + 1;
            else if (cmpint < 0)
                iTo = iThisIndex - 1;
            else
            {
                found = true;
                break;
            }
        }
        if (!found)
            index = iTo;    //prev
        else
            index = iThisIndex;
    }

    if (!found)
    {
        ulong netr = loadPage(index);
        iFrom = 1; // Needn't search the first word anymore.
        iTo = netr - 1;
        iThisIndex = 0;
        while (iFrom <= iTo)
        {
            iThisIndex = (iFrom + iTo) / 2;
            cmpint = stardictStringCompare(string, page.entries[iThisIndex].keystr);
            if (cmpint > 0)
                iFrom = iThisIndex + 1;
            else if (cmpint < 0)
                iTo = iThisIndex - 1;
            else
            {
                found = true;
                break;
            }
        }

        index *= d->entriesPerPage;
        if (!found)
            index += iFrom;    //next
        else
            index += iThisIndex;
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

quint32 wordEntrySize() const
{
    return IndexFile::wordEntrySize();
}

