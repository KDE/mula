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

        static const qint ENTR_PER_PAGE = 32;
        static const char *CACHE_MAGIC;

        QVector<quint32> wordOffset;
        QFile indexFile;
        ulong wordCount;

        char wordEntryBuf[256 + sizeof(quint32)*2]; // The length of "word_str" should be less than 256. See src/tools/DICTFILE_FORMAT.
        struct indexEntry
        {
            qlong index;
            QString keyStr;
            void assign(qlong i, const QString& str)
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

            void fill(gchar *data, gint nent, glong idx_);

            qlong index;
            pageEntry entries[ENTR_PER_PAGE];
        } page;
}

const QString offsetIndex::CACHE_MAGIC = "StarDict's Cache, Version: 0.1";

void
offsetIndex::page_t::fill(char *data, int nent, qlong index)
{
    idx = index;
    char *p = data;
    qlong len; 
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
OffsetIndex::readFirstOnPageKey(qlong pageIndex)
{
    d->indexFile.seek(d->wordoffset[pageIndx]);
    quint spage = d->wordOffset[pageIndex + 1] - d->wordOffset[pageIndex];
    d->wordentryBuf = d->indexFile.read(1*qMin(sizeof(wordEntryBuf), spage)); //TODO: check returned values, deal with word entry that strlen>255.
    return wordEntryBuf;
}

const QString
OffsetIndex::firstOnPageKey(qlonglong pageIndex)
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
            QDebug() << Q_FUNC() << QString("Mapping the file %1 failed!").arg(fileName) << endl;
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
            QDebug() << Q_FUNC() << QString("Mapping the file %1 failed!").arg(it) << endl;
            return false;
        }


        if (!out)
            continue;

        if (fwrite(CACHE_MAGIC, 1, strlen(CACHE_MAGIC), out) != strlen(CACHE_MAGIC))
            continue;
        if (fwrite(&wordoffset[0], sizeof(wordoffset[0]), wordoffset.size(), out) != wordoffset.size())
            continue;
        fclose(out);

        QDebug() << "Save to cache" << url << endl;
        return true;
    }

    return false;
}

bool
OffsetIndex::load(const QString& url, qlonglong wc, qlonglong fsize)
{
    wordcount = wc;
    qlonglong npages = (wc - 1) / ENTR_PER_PAGE + 2;
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
            QDebug() << Q_FUNC() << QString("Mapping the file %1 failed!").arg(idxfilename) << endl;
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

inline gulong offset_index::load_page(glong page_idx)
{
    gulong nentr = ENTR_PER_PAGE;
    if (page_idx == glong(wordoffset.size() - 2))
        if ((nentr = wordcount % ENTR_PER_PAGE) == 0)
            nentr = ENTR_PER_PAGE;


    if (page_idx != page.idx)
    {
        page_data.resize(wordoffset[page_idx + 1] - wordoffset[page_idx]);
        fseek(idxfile, wordoffset[page_idx], SEEK_SET);
        fread(&page_data[0], 1, page_data.size(), idxfile);
        page.fill(&page_data[0], nentr, page_idx);
    }

    return nentr;
}

const QString
offsetIndex::key(qlong index)
{
    loadPage(idx / ENTR_PER_PAGE);
    qlong indexInPage = idx % ENTR_PER_PAGE;
    wordentryOffset = page.entries[idx_in_page].off;
    wordentrySize = page.entries[idx_in_page].size;

    return page.entries[indexInPage].keystr;
}

void
offsetIndex::data(qlong index)
{
   key(index);
}

const QByteArray
offsetIndex::keyAndData(qlong index)
{
    return key(index);
}

bool
offsetIndex::lookup(const char *str, glong &idx)
{
    bool found = false;
    qlong iFrom;
    qlong iTo = m_wordOffset.size() - 2;
    qint cmpint;
    qlong iThisIndex;
    if (stardict_strcmp(str, first.keystr.c_str()) < 0)
    {
        idx = 0;
        return false;
    }
    else if (stardict_strcmp(str, real_last.keystr.c_str()) > 0)
    {
        idx = INVALID_INDEX;
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
                bFound = true;
                break;
            }
        }
        if (!bFound)
            idx = iTo;    //prev
        else
            idx = iThisIndex;
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
            cmpint = stardict_strcmp(str, page.entries[iThisIndex].keystr);
            if (cmpint > 0)
                iFrom = iThisIndex + 1;
            else if (cmpint < 0)
                iTo = iThisIndex - 1;
            else
            {
                bFound = true;
                break;
            }
        }

        idx *= ENTR_PER_PAGE;
        if (!bFound)
            idx += iFrom;    //next
        else
            idx += iThisIndex;
    }
    else
    {
        idx *= ENTR_PER_PAGE;
    }
    return found;
}

