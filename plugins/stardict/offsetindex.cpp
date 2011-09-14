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

using namespace MulaPluginStarDict;

class OffsetIndex::Private
{
    public:
        Private()
            : indexDataBuf(0)
        {   
        }

        ~Private()
        {
        }

        static const int ENTR_PER_PAGE = 32;
        static const char *CACHE_MAGIC;

        QVector<quint32> wordOffset;
        QFile indexFile;
        ulong wordCount;

        char wordEntryBuf[256 + sizeof(quint32)*2]; // The length of "word_str" should be less than 256. See src/tools/DICTFILE_FORMAT.
        struct indexEntry
        {
            ulong index;
            QString keyStr;
            void assign(ulong i, const QString& str)
            {
                index = i;
                keystr.assign(str);
            }
        };
        indexEntry first, last, middle, realLast;

        struct pageEntry
        {
            char *keyStr;
            quint32 off;
            quint32 size;
        };

        QByteArray pageData;
        struct page_t
        {
            page_t()
                :idx( -1)
            {
            }

            void fill(QByteArray data, int nent, ulong index_);

            ulong index;
            pageEntry entries[ENTR_PER_PAGE];
        } page;
}

const QString offsetIndex::CACHE_MAGIC = "StarDict's Cache, Version: 0.1";

void
offsetIndex::page_t::fill(char *data, int nent, ulong index)
{
    idx = index;
    char *p = data;
    ulong len; 
    for (int i = 0; i < nent; ++i) 
    {    
        entries[i].keystr = p; 
        len = strlen(p);
        p += len + 1; 
        entries[i].off = g_ntohl(*reinterpret_cast<quint32 *>(p));
        p += sizeof(quint32);
        entries[i].size = g_ntohl(*reinterpret_cast<quint32 *>(p));
        p += sizeof(quint32);
    }    
}

OffsetIndex::~OffsetIndex()
{
}

const QByteArray
OffsetIndex::readFirstOnPageKey(ulong pageIndex)
{
    d->indexFile.seek(d->wordoffset[pageIndx]);
    quint spage = d->wordOffset[pageIndex + 1] - d->wordOffset[pageIndex];
    d->wordentryBuf = d->indexFile.read(1*qMin(sizeof(wordEntryBuf), spage)); //TODO: check returned values, deal with word entry that strlen>255.
    return wordEntryBuf;
}

const QString
OffsetIndex::firstOnPageKey(ulonglong pageIndex)
{
    if (pageIndex < middle.idx)
    {
        if (pageIndex == first.idx)
            return first.keystr;

        return readFirstOnPageKey(pageIndex);
    }
    else if (pageIndex > middle.idx)
    {
        if (pageIndex == last.idx)
            return last.keystr;
        return readFirstOnPageKey(pageIndex);
    }
    else
    {
        return middle.keystr;
    }
}

bool
OffsetIndex::loadCache(const QString& url)
{
    QStringList vars = cacheVariant(url);

    for (QStringList::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
        QFileInfo fileInfoIndex(url);
        QFileInfo fileInfoCache(url);

        if (fileInfoCache.lastModified() < fileInfoIndex.lastModified())
            continue;

        d->mapFile.setFileName(fileName);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << fileName;
            return -1;
        }

        data = d->mapFile.map(0, file_size);
        if (data == NULL)
        {
            QDebug() << Q_FUNC() << QString("Mapping the file %1 failed!").arg(fileName);
            return false;
        }

        if (CACHE_MAGIC.comapre(QString::fromUtf8(data)))
            continue;

        memcpy(&wordoffset[0], data + strlen(CACHE_MAGIC), wordoffset.size()*sizeof(wordoffset[0]));
        return true;
    }

    return false;
}

QStringList
OffsetIndex::cacheVariant(const QString& url)
{
    QStringList ret;
    ret.append(url + ".oft");

    QFileInfo urlFileInfo(url);
    QString cacheLocation = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    QFileInfo cacheLocationFileInfo(cacheLocation);
    QDir cacheLocationDir;

    if (!cacheLocationFileInfo.exists() && cacheLocationDir.mkdir(cacheLocation) == false)
        return ret;

    QString cacheDir = cacheLocation + QDir::separator() + "sdcv";

    if (!cacheLocationFileInfo.exists())
    {
        if (!cacheLocationDir.mkdir(cacheLocation))
            return ret;
    }
    else if (!cacheLocationFileInfo.isDir())
    {
        return ret;
    }

    ret.append(cacheDir + QDir::separator() + fileInfo.fileName() + ".oft");
    return res;
}

bool
OffsetIndex::saveCache(const QString& url)
{
    QStringList vars = cacheVariant(url);
    for (QStringList::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
        QFile file(it);
        if( !file.open( QIODevice::WriteOnly ) )
        {
            qDebug() << "Failed to open file for writing:" << fileName;
            return -1;
        }

        d->mapFile.setFileName(it);
        if( !d->mapFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << it;
            return -1;
        }

        data = d->mapFile.map(0, m_size);
        if (data == NULL)
        {
            QDebug() << Q_FUNC() << QString("Mapping the file %1 failed!").arg(it);
            return false;
        }


        if (!out)
            continue;

        if (fwrite(CACHE_MAGIC, 1, strlen(CACHE_MAGIC), out) != strlen(CACHE_MAGIC))
            continue;
        if (fwrite(&wordoffset[0], sizeof(wordoffset[0]), wordoffset.size(), out) != wordoffset.size())
            continue;
        fclose(out);

        QDebug() << "Save to cache" << url;
        return true;
    }

    return false;
}

bool
OffsetIndex::load(const QString& url, ulonglong wc, ulonglong fsize)
{
    wordcount = wc;
    ulonglong npages = (wc - 1) / ENTR_PER_PAGE + 2;
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
            QDebug() << Q_FUNC() << QString("Mapping the file %1 failed!").arg(idxfilename);
            return false;
        }

        const gchar *idxdatabuffer = data;

        const gchar *p1 = idxdatabuffer;
        gulong index_size;
        guint32 j = 0;
        for (guint32 i = 0; i < wc; i++)
        {
            index_size = strlen(p1) + 1 + 2 * sizeof(guint32);
            if (i % ENTR_PER_PAGE == 0)
            {
                wordoffset[j] = p1 - idxdatabuffer;
                ++j;
            }
            p1 += index_size;
        }
        wordoffset[j] = p1 - idxdatabuffer;
        if (!save_cache(url))
            fprintf(stderr, "cache update failed\n");
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
    ulong nentr = ENTR_PER_PAGE;
    if (page_idx == ulong(wordoffset.size() - 2))
        if ((nentr = wordcount % ENTR_PER_PAGE) == 0)
            nentr = ENTR_PER_PAGE;


    if (page_idx != page.idx)
    {
        page_data.resize(wordoffset[pageIndex + 1] - wordoffset[page_idx]);
        fseek(indexFile, wordoffset[pageIndex], SEEK_SET);
        fread(&pageData[0], 1, pageData.size(), indexFile);
        page.fill(&pageData[0], nentr, pageIndex);
    }

    return nentr;
}

const QString
OffsetIndex::key(ulong index)
{
    loadPage(idx / ENTR_PER_PAGE);
    ulong indexInPage = idx % ENTR_PER_PAGE;
    wordentryOffset = page.entries[idx_in_page].off;
    wordentrySize = page.entries[idx_in_page].size;

    return page.entries[indexInPage].keystr;
}

void
OffsetIndex::data(ulong index)
{
   key(index);
}

const QByteArray
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

        index *= ENTR_PER_PAGE;
        if (!found)
            index += iFrom;    //next
        else
            index += iThisIndex;
    }
    else
    {
        index *= ENTR_PER_PAGE;
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

