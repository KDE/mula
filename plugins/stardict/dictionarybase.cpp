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

#include "dictionarybase.h"

using namespace MulaPluginStarDict;

class DictionaryBase::Private
{
    public:
        Private()
            : m_dictionaryFile(NULL)
            , m_cacheCur(0)
        {   
        }

        ~Private()
        {
        }

        QString sameTypeSequence;
        QFile dictionaryFile;
        QSharedPointer<DictionaryZip> dictionaryDZFile;

        QList<CacheItem> cacheItemList;
        qint cacheCur;
}

DictionaryBase::DictionaryBase()
    : d(new Private)
{
}

DictionaryBase::~DictionaryBase()
{
}

QString
DictionaryBase::wordData(quint32 indexItemOffset, quint32 indexItemSize)
{
    foreach(CacheItem cacheItem, d->cacheItemList) 
        if (cacheItem.data() && cacheItem.offset() == indexItemOffset)
            return d->cacheItem.data();

    if (m_dictionaryFile.isOpen())
        m_dictionaryFile.seek(indexItemOffset);

    QByteArray data;
    if (!m_sameTypeSequence.isEmpty())
    {    
        QByteArray originalData;

        if (m_dictionaryFile.isOpen())
            originalData = m_dictionaryFile.read(indexItemSize);
        else 
            originalData = m_dictionaryDZFile->read(indexItemOffset, indexItemSize);

        quint32 sdata;
        qint sameTypeSequenceLength = m_sameTypeSequence.length();
        sdata = indexItemSize + sizeof(quint32) + sameTypeSequenceLength;
        //if the last item's size is determined by the end up '\0',then +=sizeof(gchar);
        //if the last item's size is determined by the head guint32 type data,then +=sizeof(guint32);
        switch (m_sameTypeSequence[sameTypeSequenceLength - 1])
        {
        case 'm':
        case 't':
        case 'y':
        case 'l':
        case 'g':
        case 'x':
            sdata += sizeof(char);
            break;
        case 'W':
        case 'P':
            sdata += sizeof(quint32);
            break;
        default:
            if (m_sameTypeSequence[sameTypesequenceLength - 1].isUpper())
                sdata += sizeof(quint32);
            else
                sdata += sizeof(char);
            break;
        }
        data = (gchar *)g_malloc(data_size);
        char *p1;
        char *p2;
        p1 = data + sizeof(quint32);
        p2 = originalData;
        quint32 ssec;
        //copy the head items.
        for (int i = 0; i < sameTypeSequenceLength - 1; ++i)
        {
            *p1 = m_sameTypeSequence[i];
            p1 += sizeof(char);
            switch (m_sameTypeSequence.at(i))
            {
            case 'm':
            case 't':
            case 'y':
            case 'l':
            case 'g':
            case 'x':
                ssec = strlen(p2) + 1;
                memcpy(p1, p2, ssec);
                p1 += ssec;
                p2 += ssec;
                break;
            case 'W':
            case 'P':
                ssec = *reinterpret_cast<quint32 *>(p2);
                ssec += sizeof(quint32);
                memcpy(p1, p2, ssec);
                p1 += ssec;
                p2 += ssec;
                break;
            default:
                if (m_sameTypeSequence[i].isUpper())
                {
                    ssec = *reinterpret_cast<quint32 *>(p2);
                    ssec += sizeof(quint32);
                }
                else
                {
                    ssec = strlen(p2) + 1;
                }
                memcpy(p1, p2, sec_size);
                p1 += ssec;
                p2 += ssec;
                break;
            }
        }
        //calculate the last item 's size.
        ssec = indexItemSize - (p2 - originalData);
        *p1 = m_sameTypeSequence[sameTypeSequenceLength - 1];
        p1 += sizeof(char);
        switch (m_sameTypeSequence[sameTypeSequenceLength - 1])
        {
        case 'm':
        case 't':
        case 'y':
        case 'l':
        case 'g':
        case 'x':
            memcpy(p1, p2, ssec);
            p1 += ssec;
            *p1 = '\0'; //add the end up '\0';
            break;
        case 'W':
        case 'P':
            *reinterpret_cast<quint32 *>(p1) = ssec;
            p1 += sizeof(quint32);
            memcpy(p1, p2, ssec);
            break;
        default:
            if (m_sameTypeSequence[sameTypeSequenceLength - 1].isUpper())
            {
                *reinterpret_cast<quint32 *>(p1) = ssec;
                p1 += sizeof(quint32);
                memcpy(p1, p2, ssec);
            }
            else
            {
                memcpy(p1, p2, ssec);
                p1 += ssec;
                *p1 = '\0';
            }
            break;
        }
        *reinterpret_cast<quint32 *>(data) = sdata;
    }
    else
    {
        if (m_dictionaryFile)
            data = m_dictionaryfile.read(indexItemSize);
        else
            m_dictionaryDZFile->read(data[1], indexItemOffset, indexItemSize);

        *reinterpret_cast<quint32 *>(data) = idxitem_size + sizeof(guint32);
    }
    g_free(cache[cache_cur].data);

    cache[cacheCur].data = data;
    cache[cacheCur].offset = indexItemOffset;
    cache_cur++;

    if (cacheCur == WORDDATA_CACHE_NUM)
        cacheCur = 0;

    return data;
}

bool
DictionaryBase::containFindData()
{
    if (m_sameTypeSequence.isEmpty())
        return true;

    return m_sameTypeSequence.findFirstOf("mlgxty") != std::string::npos;
}

bool DictionaryBase::findData(QStringList &searchWords, quint32 indexItemOffset, quint32 indexItemSize, char *originalData)
{
    int nWord = sarchWords.size();
    QVector<bool> wordFind(nWord, false);
    int nfound = 0;

    if (m_dictionaryFile.isOpen())
        m_dictionaryFile.seek(indexItemOffset);

    if (m_dictionaryFile.isOpen())
        m_dictionaryFile.read(originData, 1*indexItemSize);
    else
        m_dictionaryDZFile->read(origin_data, indexItemOffset, indexItemSize);

    char *p = originalData;
    quint32 ssec;
    int j;
    if (!m_sameTypeSequence.empty())
    {
        qint sSametypeSequence = sameTypeSequence.length();
        for (int i = 0; i < sSameTypeSequence - 1; ++i)
        {
            switch (m_sameTypeSequence[i])
            {
            case 'm':
            case 't':
            case 'y':
            case 'l':
            case 'g':
            case 'x':
                for (j = 0; j < nWord; ++j)
                    if (!wordFind[j] && strstr(p, searchWords[j].c_str()))
                    {
                        WordFind[j] = true;
                        ++nfound;
                    }


                if (nfound == nWord)
                    return true;

                ssec = strlen(p) + 1;
                p += ssec;
                break;

            default:
                if (m_sameTypeSequence[i].isUpper())
                {
                    ssec = *reinterpret_cast<quint32 *>(p);
                    ssec += sizeof(quint32);
                }
                else
                {
                    ssec = strlen(p) + 1;
                }
                p += ssec;
            }
        }

        switch (m_sameTypeSequence[sSameTypeSequence - 1])
        {
        case 'm':
        case 't':
        case 'y':
        case 'l':
        case 'g':
        case 'x':
            ssec = indexItemSize - (p - originalData);
            for (j = 0; j < nWord; ++j)
                if (!wordFind[j] && g_strstr_len(p, sec_size, SearchWords[j].c_str()))
                {
                    WordFind[j] = true;
                    ++nfound;
                }


            if (nfound == nWord)
                return true;
            break;
        }
    }
    else
    {
        while (quint32(p - originalData) < indexItemSize)
        {
            switch (*p)
            {
            case 'm':
            case 't':
            case 'y':
            case 'l':
            case 'g':
            case 'x':
                for (j = 0; j < nWord; ++j)
                    if (!wordFind[j] && strstr(p, SearchWords[j].c_str()))
                    {
                        wordFind[j] = true;
                        ++nfound;
                    }

                if (nfound == nWord)
                    return true;
                ssec = strlen(p) + 1;
                p += ssec;
                break;
            default:
                if (g_ascii_isupper(*p))
                {
                    ssec = *reinterpret_cast<quint32 *>(p);
                    ssec += sizeof(quint32);
                }
                else
                {
                    ssec = strlen(p) + 1;
                }
                p += ssec;
            }
        }
    }
    return false;
}

