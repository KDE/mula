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
        {
        }

        ~Private()
        {
        }

        StarDictDictionaryInfo dictionaryInfo;
        QScopedPointer<AbstractIndexFile> indexFile;
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
    return d->dictionaryInfo.wordCount();
}

QString
Dictionary::dictionaryName() const
{
    return d->dictionaryInfo.bookName();
}

QString
Dictionary::ifoFilePath() const
{
    return d->dictionaryInfo.ifoFilePath();
}

QString
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

    return AbstractDictionary::wordData(d->indexFile->wordEntryOffset(), d->indexFile->wordEntrySize());
}

WordEntry
Dictionary::wordEntry(long index)
{
    if (d->indexFile.isNull())
        return WordEntry();

    WordEntry wordEntry;
    wordEntry.setData(d->indexFile->key(index));
    wordEntry.setDataOffset(d->indexFile->wordEntryOffset());
    wordEntry.setDataSize(d->indexFile->wordEntrySize());

    return wordEntry;
}

int
Dictionary::lookup(const QString& word)
{
    if (d->indexFile.isNull())
        return -1;

    return d->indexFile->lookup(word.toUtf8());
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
        completeFilePath.chop(sizeof(".dz"));

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
        completeFilePath.chop(sizeof(".gz"));
        d->indexFile.reset(new OffsetCacheFile);
    }

    if (!d->indexFile->load(completeFilePath))
        return false;

    return true;
}

bool
Dictionary::loadIfoFile(const QString& ifoFilePath)
{
    if (!d->dictionaryInfo.loadFromIfoFile(ifoFilePath))
        return false;

    if (d->dictionaryInfo.wordCount() == 0)
        return false;

    setSameTypeSequence(d->dictionaryInfo.sameTypeSequence());

    return true;
}

QVector<int>
Dictionary::lookupPattern(const QString& pattern, int maximumIndexListSize)
{
    QVector<int> indexList;

    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);

    for (int i = 0; i < articleCount() && indexList.size() < maximumIndexListSize - 1; ++i)
    {
        if (rx.exactMatch(key(i)))
            indexList.append(i);
    }

    return indexList;
}

