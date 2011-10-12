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

#include "abstractdictionary.h"

#include "wordentry.h"
#include "dictionaryzip.h"

#include <QtCore/QFile>
#include <QtCore/QVector>

using namespace MulaPluginStarDict;

class AbstractDictionary::Private
{
    public:
        Private()
            : dictionaryFile(new QFile)
            , compressedDictionaryFile(0)
            , currentCacheItemIndex(0)
        {
        }

        ~Private()
        {
        }

        QString sameTypeSequence;
        QFile *dictionaryFile;
        DictionaryZip *compressedDictionaryFile;

        QList<WordEntry> cacheItemList;
        int currentCacheItemIndex;
        static const int wordDataCacheSize = 10;
};

AbstractDictionary::AbstractDictionary()
    : d(new Private)
{
    d->cacheItemList.reserve(d->wordDataCacheSize);
}

AbstractDictionary::~AbstractDictionary()
{
    delete d->compressedDictionaryFile;
    delete d->dictionaryFile;
    delete d;
}

const QByteArray
AbstractDictionary::wordData(quint32 indexItemOffset, qint32 indexItemSize)
{
    // Check first whether or not the data is already available in the cache
    foreach(const WordEntry& cacheItem, d->cacheItemList)
    {
        if (!cacheItem.data().isEmpty() && cacheItem.dataOffset() == indexItemOffset)
            return cacheItem.data();
    }

    QByteArray resultData;
    QByteArray originalData;

    if (!d->sameTypeSequence.isEmpty())
    {
        if (d->dictionaryFile->isOpen())
        {
            d->dictionaryFile->seek(indexItemOffset);
            originalData = d->dictionaryFile->read(indexItemSize);
        }
        else
        {
            originalData = d->compressedDictionaryFile->read(indexItemOffset, indexItemSize);
        }

        int sameTypeSequenceLength = d->sameTypeSequence.length();

        int sectionSize = 0;
        int sectionPosition = 0;

        //copy the head items.
        foreach (const QChar& ch, d->sameTypeSequence.left(sameTypeSequenceLength - 1))
        {
            if (ch.isUpper())
            {
                sectionSize = *reinterpret_cast<quint32 *>(originalData.mid(sectionPosition).data());
                sectionSize += sizeof(quint32);
            }
            else
            {
                sectionSize = qstrlen(originalData.mid(sectionPosition)) + 1;
            }

            resultData.append(ch);
            resultData.append(originalData.mid(sectionPosition, sectionSize));
            sectionPosition += sectionSize;
        }

        // Calculate the last item's size.
        sectionSize = indexItemSize - sectionPosition;
        resultData.append(d->sameTypeSequence.at(sameTypeSequenceLength - 1));
        if (d->sameTypeSequence.at(sameTypeSequenceLength - 1).isUpper())
        {
            resultData.append(reinterpret_cast<char*>(&sectionSize), sizeof(quint32));
            resultData.append(originalData.mid(sectionPosition, sectionSize));
        }
        else
        {
            resultData.append(originalData.mid(sectionPosition, sectionSize));
            sectionPosition += sectionSize;
            resultData.append('\0');
        }
    }
    else
    {
        if (d->dictionaryFile->isOpen())
            originalData = d->dictionaryFile->read(indexItemSize);
        else
            originalData = d->compressedDictionaryFile->read(indexItemOffset, indexItemSize);

        resultData = originalData;
    }

    d->cacheItemList[d->currentCacheItemIndex].setData(resultData);
    d->cacheItemList[d->currentCacheItemIndex].setDataOffset(indexItemOffset);
    ++d->currentCacheItemIndex;

    if (d->currentCacheItemIndex == d->wordDataCacheSize)
        d->currentCacheItemIndex = 0;

    return resultData;
}

bool
AbstractDictionary::containFindData()
{
    if (d->sameTypeSequence.isEmpty())
        return true;

    foreach (const QChar& ch, QString("mlgxty"))
    {
        if (d->sameTypeSequence.contains(ch))
            return true;
    }

    return false;
}

bool
AbstractDictionary::findData(const QStringList &searchWords, qint32 indexItemOffset, qint32 indexItemSize, QByteArray& originalData)
{
    int wordCount = searchWords.size();
    QVector<bool> wordFind(wordCount, false);

    if (d->dictionaryFile->isOpen())
    {
        d->dictionaryFile->seek(indexItemOffset);
        originalData = d->dictionaryFile->read(indexItemSize);
    }
    else
    {
        originalData = d->compressedDictionaryFile->read(indexItemOffset, indexItemSize);
    }

    int sectionSize = 0;
    int sectionPosition = 0;
    int foundCount = 0;

    if (!d->sameTypeSequence.isEmpty())
    {
        int sameTypeSequenceLength = d->sameTypeSequence.length();
        foreach (const QChar& ch, d->sameTypeSequence.left(sameTypeSequenceLength - 1))
        {
            switch (ch.toAscii())
            {
            case 'm':
            case 'l':
            case 'g':
            case 't':
            case 'x':
            case 'y':
                for (int j = 0; j < wordCount; ++j)
                {
                    if (!wordFind.at(j) && originalData.indexOf(searchWords.at(j), sectionPosition) > -1)
                    {
                        wordFind[j] = true;
                        ++foundCount;
                    }
                }

                // If everything has been found
                if (foundCount == wordCount)
                    return true;

                sectionSize = qstrlen(originalData.mid(sectionPosition)) + 1;
                sectionPosition += sectionSize;
                break;

            default:
                if (ch.isUpper())
                {
                    sectionSize = *reinterpret_cast<quint32 *>(originalData.mid(sectionPosition).data());
                    sectionSize += sizeof(quint32);
                }
                else
                {
                    sectionSize = qstrlen(originalData.mid(sectionPosition)) + 1;
                }

                sectionPosition += sectionSize;
            }
        }

        switch (d->sameTypeSequence.at(sameTypeSequenceLength - 1).toAscii())
        {
        case 'm':
        case 'l':
        case 'g':
        case 't':
        case 'x':
        case 'y':
            sectionSize = indexItemSize - sectionSize;
            for (int j = 0; j < wordCount; ++j)
            {
                if (!wordFind[j] && originalData.indexOf(searchWords.at(j), sectionPosition) > -1)
                {
                    wordFind[j] = true;
                    ++foundCount;
                }
            }

            if (foundCount == wordCount)
                return true;

            break;
        }
    }
    else
    {
        while (sectionPosition < indexItemSize)
        {
            switch (originalData.at(sectionPosition))
            {
            case 'm':
            case 'l':
            case 'g':
            case 't':
            case 'x':
            case 'y':
                for (int j = 0; j < wordCount; ++j)
                {
                    if (!wordFind.at(j) && originalData.indexOf(searchWords.at(j), sectionSize) > -1)
                    {
                        wordFind[j] = true;
                        ++foundCount;
                    }
                }

                // Everything has been found
                if (foundCount == wordCount)
                    return true;

                sectionSize = qstrlen(originalData.mid(sectionPosition)) + 1;

                break;
            default:
                if (QChar(originalData.at(sectionSize)).isUpper())
                {
                    originalData.fromRawData(reinterpret_cast<char*>(&sectionSize), 4);
                    sectionSize += sizeof(quint32);
                }
                else
                {
                    sectionSize = qstrlen(originalData.mid(sectionPosition)) + 1;
                }
            }

            sectionPosition += sectionSize;
        }
    }

    return false;
}

DictionaryZip*
AbstractDictionary::compressedDictionaryFile() const
{
    return d->compressedDictionaryFile;
}

QFile*
AbstractDictionary::dictionaryFile() const
{
    return d->dictionaryFile;
}

QString&
AbstractDictionary::sameTypeSequence() const
{
    return d->sameTypeSequence;
}

void
AbstractDictionary::setSameTypeSequence(const QString& sameTypeSequence)
{
    d->sameTypeSequence = sameTypeSequence;
}
