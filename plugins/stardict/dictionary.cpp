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

#include "dictionary.h"

using namespace MulaPluginStarDict;

class Dictionary::Private
{
    public:
        Private()
            : wordCount(0)
        {   
        }

        ~Private()
        {
        }
 
        QString ifoFileName;
        ulong wordCount;
        QString bookName;

        QScopedPointer<IndexFile> indexFile;
}

Dictionary::Dictionary()
    : d(new Private)
{
}

Dictoinary::~Dictionary()
{
}

ulong
Dictionary::articlesCount() const
{   
    return d->wordCount;
}   

const QString&
Dictionary::dictionaryName() const
{   
    return d->bookname;
}   

const QString&
Dictionary::ifoFileName() const
{   
    return d->ifoFileName;
}   

const QString&
Dictionary::key(qlong index) const
{   
    return d->indexFile->key(index);
}   

QString
Dictionary::data(qlong index)
{
    d->indexFile->data(index);
    return DictionaryBase::wordData(d->indexFile->wordEntryOffset, d->indexFile->wordEntrySize);
}

void
Dictionary::keyAndData(qlong index, const QStringList key, quint32 *offset, quint32 *size)
{
    *key = d->indexFile->keyAndData(index);
    *offset = d->indexFile->wordEntryOffset;
    *size = d->indexFile->wordEntrySize;
}

bool
Dictionary::lookup(const QString str, qlong &index)
{
    return d->indexFile->lookup(str, index);
}

bool
Dictionary::load(const QString& ifoFilePath)
{
    if (!loadIfoFile(ifoFilePath))
        return false;

    QString completeFilePath = ifoFilePath;
    completeFilePath.replace(completeFilePath.length() - sizeof("ifo") + 1, sizeof("ifo") - 1, "dict.dz");

    if (completeFilePath.exists())
    {
        d->dictionaryDZFile.reset(new DictionaryZip);
        if (!d->dictionaryDZFile->open(completeFilePath, 0))
        {
            qDebug() << "Failed to open file:" << completeFilePath;
            return false;
        }
    }
    else
    {
        completeFilePath.remove(completeFilePath.length() - sizeof(".dz") + 1, sizeof(".dz") - 1);
        d->dictionaryFile(completeFilePath);
        if( !d->dictionaryFile.open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << completeFilePath;
            return -1;
        }
    }

    completeFilePath = ifoFilePath;
    completeFilePath.replace(completeFilePath.length() - sizeof("ifo") + 1, sizeof("ifo") - 1, "idx.gz");

    if (completeFilePath.exists())
    {
        d->indexFile.reset(new wordListIndex);
    }
    else
    {
        completeFilePath.remove(completeFilePath.length() - sizeof(".gz") + 1, sizeof(".gz") - 1);
        d->indexFile.reset(new offsetIndex);
    }

    if (!d->indexFile->load(completeFilePath, wordCount, indexFileSize))
        return false;

    return true;
}

bool
Dictionary::loadIfoFile(const QString& ifoFileName)
{
    DictonaryInfo dictionaryInfo;
    if (!dictionaryInfo.loadFromIfoFile(ifoFileName, false))
        return false;

    if (dictionaryInfo.wordcount() == 0)
        return false;

    d->ifoFileName = dictionaryInfo.ifoFileName();
    d->wordcount = dictionaryInfo.wordcount();
    d->bookname = dictionaryInfo.bookname();

    d->indexFileSize = dictionaryInfo.indexFileSize();

    d->sameTypeSequence = dictionaryInfo.sameTypeSequence();

    return true;
}

bool
Dictionary::lookupWithRule(const QString *pattern, qlong *aIndex, int iBuffLen)
{
    int indexCount = 0;

    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);

    for (int i = 0; i < narticles() && indexCount < iBuffLen - 1; ++i)
        if (rx.exactMatch(key(i)))
            aIndex[indexCount++] = i;

    aIndex[indexCount] = -1; // -1 is the end.

    return (indexCount > 0);
}

