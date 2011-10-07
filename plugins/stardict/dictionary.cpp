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
 * License aLong with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "dictionary.h"

#include "dictionaryzip.h"
#include "stardictdictionaryinfo.h"
#include "indexfile.h"
#include "offsetcachefile.h"

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

        QString ifoFilePath;
        long wordCount;
        QString bookName;

        QScopedPointer<AbstractIndexFile> indexFile;
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
Dictionary::articleCount() const
{
    return d->wordCount;
}

const QString
Dictionary::dictionaryName() const
{
    return d->bookName;
}

const QString
Dictionary::ifoFilePath() const
{
    return d->ifoFilePath;
}

const QString
Dictionary::key(long index) const
{
    if (d->indexFile.isNull())
        return QString();

    return d->indexFile->key(index);
}

QString
Dictionary::data(long index)
{
    Q_UNUSED(index);
    if (d->indexFile.isNull())
        return QString();

    return DictionaryBase::wordData(d->indexFile->wordEntryOffset(), d->indexFile->wordEntrySize());
}

void
Dictionary::keyAndData(long index, QByteArray key, qint32 &offset, qint32 &size)
{
    key = d->indexFile->key(index);
    offset = d->indexFile->wordEntryOffset();
    size = d->indexFile->wordEntrySize();
}

bool
Dictionary::lookup(const QString word, long &index)
{
    if (d->indexFile.isNull())
        return false;

    return d->indexFile->lookup(word.toUtf8(), index);
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
        if (uncompressedDictionaryFile)
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
        d->indexFile.reset(new IndexFile);
    }
    else
    {
        completeFilePath.remove(completeFilePath.length() - sizeof(".gz") + 1, sizeof(".gz") - 1);
        d->indexFile.reset(new OffsetCacheFile);
    }

    if (!d->indexFile->load(completeFilePath, d->wordCount, d->indexFileSize))
        return false;

    return true;
}

bool
Dictionary::loadIfoFile(const QString& ifoFilePath)
{
    StarDictDictionaryInfo dictionaryInfo;
    if (!dictionaryInfo.loadFromIfoFile(ifoFilePath, false))
        return false;

    if (dictionaryInfo.wordCount() == 0)
        return false;

    d->ifoFilePath = dictionaryInfo.ifoFilePath();
    d->wordCount = dictionaryInfo.wordCount();
    d->bookName = dictionaryInfo.bookName();

    d->indexFileSize = dictionaryInfo.indexFileSize();

    setSameTypeSequence(dictionaryInfo.sameTypeSequence());

    return true;
}

bool
Dictionary::lookupWithRule(const QString& pattern, long *aIndex, int iBuffLen)
{
    int indexCount = 0;

    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);

    for (int i = 0; i < articleCount() && indexCount < iBuffLen - 1; ++i)
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
