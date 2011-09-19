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

#include "dictionarybase.h"

#include "cacheitem.h"
#include "dictionaryzip.h"

#include <QtCore/QFile>
#include <QtCore/QVector>

// Word's pure text meaning.
// The data should be a utf-8 string ending with '\0'.
// case 'm':

// Word's pure text meaning.
// The data is NOT a utf-8 string, but is instead a string in locale
// encoding, ending with '\0'.  Sometimes using this type will save disk
// space, but its use is discouraged.
// case 'l':

// English phonetic string.
// The data should be a utf-8 string ending with '\0'.
// case 't':

// Chinese YinBiao or Japanese KANA.
// The data should be a utf-8 string ending with '\0'.
// case 'y':

// A utf-8 string which is marked up with the Pango text markup language.
// For more information about this markup language, See the "Pango
// Reference Manual."
// You might have it installed locally at:
// file:///usr/share/gtk-doc/html/pango/PangoMarkupFormat.html
// case 'g':

// A utf-8 string which is marked up with the xdxf language.
// See http://xdxf.sourceforge.net
// StarDict have these extention:
// <rref> can have "type" attribute, it can be "image", "sound", "video" 
// and "attach".
// <kref> can have "k" attribute.
// case 'x':

// wav file.
// The data begins with a network byte-ordered guint32 to identify the wav
// file's size, immediately followed by the file's content.
// case 'W':

// Picture file.
// The data begins with a network byte-ordered guint32 to identify the picture
// file's size, immediately followed by the file's content.
// case 'P':
        
using namespace MulaPluginStarDict;

class DictionaryBase::Private
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

        QList<CacheItem> cacheItemList;
        int currentCacheItemIndex;
};

DictionaryBase::DictionaryBase()
    : d(new Private)
{
    d->cacheItemList.reserve(WORDDATA_CACHE_NUM);
}

DictionaryBase::~DictionaryBase()
{
    delete d->compressedDictionaryFile;
    delete d->dictionaryFile;
    delete d;
}

const QByteArray
DictionaryBase::wordData(quint32 indexItemOffset, qint32 indexItemSize)
{
    foreach(CacheItem cacheItem, d->cacheItemList)
    {
        if (!cacheItem.data().isEmpty() && cacheItem.offset() == indexItemOffset)
            return cacheItem.data();
    }

    if (d->dictionaryFile->isOpen())
        d->dictionaryFile->seek(indexItemOffset);

    QByteArray resultData;
    QByteArray originalData;

    if (!d->sameTypeSequence.isEmpty())
    {    
        if (d->dictionaryFile->isOpen())
            originalData = d->dictionaryFile->read(indexItemSize);
        else 
            originalData = d->compressedDictionaryFile->read(indexItemOffset, indexItemSize);

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

            resultData.append(originalData.mid(sectionPosition, sectionSize));
            sectionPosition += sectionSize;
        }

        // Calculate the last item's size.
        sectionSize = indexItemSize - sectionPosition;
        if (d->sameTypeSequence[sameTypeSequenceLength - 1].isUpper())
        {
            originalData.fromRawData(reinterpret_cast<char*>(&sectionSize), 4);
            sectionSize += sizeof(quint32);
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
    }

    d->cacheItemList[d->currentCacheItemIndex].setData(resultData);
    d->cacheItemList[d->currentCacheItemIndex].setOffset(indexItemOffset);
    ++d->currentCacheItemIndex;

    if (d->currentCacheItemIndex == WORDDATA_CACHE_NUM)
        d->currentCacheItemIndex = 0;

    return resultData;
}

bool
DictionaryBase::containFindData()
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

bool DictionaryBase::findData(const QStringList &searchWords, qint32 indexItemOffset, qint32 indexItemSize, QByteArray& originalData)
{
    int wordCount = searchWords.size();
    QVector<bool> wordFind(wordCount, false);

    if (d->dictionaryFile->isOpen())
        d->dictionaryFile->seek(indexItemOffset);

    if (d->dictionaryFile->isOpen())
        originalData = d->dictionaryFile->read(indexItemSize);
    else
        originalData = d->compressedDictionaryFile->read(indexItemOffset, indexItemSize);

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
            case 't':
            case 'y':
            case 'l':
            case 'g':
            case 'x':
                for (int j = 0; j < wordCount; ++j)
                {
                    if (!wordFind.at(j) && originalData.indexOf(searchWords.at(j), sectionPosition) == -1)
                    {
                        wordFind[j] = true;
                        ++foundCount;
                    }
                }

                // If everything has been found
                if (foundCount == wordCount)
                    return true;

                sectionSize = originalData.indexOf(QChar('\0'), sectionPosition) + 1;
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
                    sectionSize = originalData.indexOf(QChar('\0'), sectionPosition) + 1;
                }

                sectionPosition += sectionSize;
            }
        }

        switch (d->sameTypeSequence.at(sameTypeSequenceLength - 1).toAscii())
        {
        case 'm':
        case 't':
        case 'y':
        case 'l':
        case 'g':
        case 'x':
            sectionSize = indexItemSize - sectionSize;
            for (int j = 0; j < wordCount; ++j)
            {
                if (!wordFind[j] && originalData.contains(searchWords.at(j).mid(sectionSize).toUtf8()))
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
        for (; sectionPosition < indexItemSize; sectionPosition += sectionSize)
        {
            switch (originalData.at(sectionPosition))
            {
            case 'm':
            case 't':
            case 'y':
            case 'l':
            case 'g':
            case 'x':
                for (int j = 0; j < wordCount; ++j)
                {
                    if (!wordFind.at(j) && originalData.indexOf(searchWords.at(j), sectionSize) == -1)
                    {
                        wordFind[j] = true;
                        ++foundCount;
                    }
                }

                // If everything has been found
                if (foundCount == wordCount)
                    return true;

                sectionSize = originalData.indexOf(QChar('\0'), sectionPosition) + 1;

                break;
            default:
                if (QChar(originalData.at(sectionSize)).isUpper())
                {
                    originalData.fromRawData(reinterpret_cast<char*>(&sectionSize), 4); 
                    sectionSize += sizeof(quint32);
                }
                else
                {
                    sectionSize = originalData.indexOf(QChar('\0'), sectionPosition) + 1;
                }
            }
        }
    }

    return false;
}

DictionaryZip*
DictionaryBase::compressedDictionaryFile() const
{
    return d->compressedDictionaryFile;
}

QString&
DictionaryBase::sameTypeSequence() const
{
    return d->sameTypeSequence;
}

void
DictionaryBase::setSameTypeSequence(const QString& sameTypeSequence)
{
    d->sameTypeSequence = sameTypeSequence;
}
