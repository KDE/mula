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

#include "dictionaryzip.h"
#include "dictionaryinfo.h"
#include "indexfile.h"
#include "wordlistindex.h"
#include "offsetindex.h"

#include <QtCore/QScopedPointer>
#include <QtCore/QFile>
#include <QtCore/QDebug>

using namespace MulaPluginStarDict;

class Dictionary::Private
{
    public:
        Private()
            : wordCount(0)
            , indexFileSize(0)
        {   
        }

        ~Private()
        {
        }
 
        QString ifoFileName;
        ulong wordCount;
        QString bookName;

        QScopedPointer<IndexFile> indexFile;
        int indexFileSize;
};

Dictionary::Dictionary()
    : d(new Private)
{
}

Dictionary::~Dictionary()
{
}

int
Dictionary::articlesCount() const
{   
    return d->wordCount;
}   

const QString&
Dictionary::dictionaryName() const
{   
    return d->bookName;
}   

const QString&
Dictionary::ifoFileName() const
{   
    return d->ifoFileName;
}   

const QString&
Dictionary::key(ulong index) const
{   
    return d->indexFile->key(index);
}   

const QString
Dictionary::data(ulong index)
{
    d->indexFile->data(index);
    return DictionaryBase::wordData(d->indexFile->wordEntryOffset(), d->indexFile->wordEntrySize());
}

void
Dictionary::keyAndData(ulong index, QString& key, quint32 *offset, quint32 *size)
{
    key = d->indexFile->keyAndData(index);
    *offset = d->indexFile->wordEntryOffset();
    *size = d->indexFile->wordEntrySize();
}

bool
Dictionary::lookup(const QString string, ulong &index)
{
    return d->indexFile->lookup(string, index);
}

bool
Dictionary::load(const QString& ifoFilePath)
{
    if (!loadIfoFile(ifoFilePath))
        return false;

    QString completeFilePath = ifoFilePath;
    completeFilePath.replace(completeFilePath.length() - sizeof("ifo") + 1, sizeof("ifo") - 1, "dict.dz");

    if (QFile(completeFilePath).exists())
    {
        DictionaryZip *dictionaryZip = compressedDictionaryFile();
        if (!dictionaryZip)
        {
            delete dictionaryZip;
            dictionaryZip = 0;
        }
        
        dictionaryZip = new DictionaryZip();
        if (!dictionaryZip->open(completeFilePath, 0))
        {
            qDebug() << "Failed to open file:" << completeFilePath;
            return false;
        }
    }
    else
    {
        completeFilePath.remove(completeFilePath.length() - sizeof(".dz") + 1, sizeof(".dz") - 1);

        QFile *uncompressedDictionaryFile = dictionaryFile();
        if (!uncompressedDictionaryFile)
        {
            delete uncompressedDictionaryFile;
            uncompressedDictionaryFile = 0;
        }
        
        uncompressedDictionaryFile = new QFile(completeFilePath);
        if( uncompressedDictionaryFile->open( QIODevice::ReadOnly ) )
        {
            qDebug() << "Failed to open file:" << completeFilePath;
            return -1;
        }
    }

    completeFilePath = ifoFilePath;
    completeFilePath.replace(completeFilePath.length() - sizeof("ifo") + 1, sizeof("ifo") - 1, "idx.gz");

    if (QFile(completeFilePath).exists())
    {
        d->indexFile.reset(new WordListIndex);
    }
    else
    {
        completeFilePath.remove(completeFilePath.length() - sizeof(".gz") + 1, sizeof(".gz") - 1);
        d->indexFile.reset(new OffsetIndex);
    }

    if (!d->indexFile->load(completeFilePath, d->wordCount, d->indexFileSize))
        return false;

    return true;
}

bool
Dictionary::loadIfoFile(const QString& ifoFileName)
{
    DictionaryInfo dictionaryInfo;
    if (!dictionaryInfo.loadFromIfoFile(ifoFileName, false))
        return false;

    if (dictionaryInfo.wordCount() == 0)
        return false;

    d->ifoFileName = dictionaryInfo.ifoFileName();
    d->wordCount = dictionaryInfo.wordCount();
    d->bookName = dictionaryInfo.bookName();

    d->indexFileSize = dictionaryInfo.indexFileSize();

    setSameTypeSequence(dictionaryInfo.sameTypeSequence());

    return true;
}

bool
Dictionary::lookupWithRule(const QString& pattern, ulong *aIndex, int iBuffLen)
{
    int indexCount = 0;

    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);

    for (int i = 0; i < articlesCount() && indexCount < iBuffLen - 1; ++i)
        if (rx.exactMatch(key(i)))
            aIndex[indexCount++] = i;

    aIndex[indexCount] = -1; // -1 is the end.

    return (indexCount > 0);
}

DictionaryZip*
Dictionary::compressedDictionaryFile() const
{
    return DictionaryBase::compressedDictionaryFile();
}

QFile*
Dictionary::dictionaryFile() const
{
    return DictionaryBase::dictionaryFile();
}

QString
Dictionary::sameTypeSequence() const
{
    return DictionaryBase::sameTypeSequence();
}

void
Dictionary::setSameTypeSequence(const QString& sameTypeSequence)
{
    DictionaryBase::setSameTypeSequence(sameTypeSequence);
}