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

#include "stardictdictionaryinfo.h"

#include <QtCore/QFile>

using namespace MulaPluginStarDict;

class StarDictDictionaryInfo::Private
{
    public:
        Private()
            : wordCount(0)
            , indexFileSize(0)
            , indexOffsetBits(0)
        {   
        }

        ~Private()
        {
        }
 
        QString ifoFilePath;
        quint32 wordCount;
        QString bookName;
        QString author;
        QString email;
        QString website;
        QString date;
        QString description;
        quint32 indexFileSize;
        quint32 indexOffsetBits;
        QString sameTypeSequence;
};

StarDictDictionaryInfo::StarDictDictionaryInfo()
    : d(new Private)
{
}

StarDictDictionaryInfo::~StarDictDictionaryInfo()
{
}

bool
StarDictDictionaryInfo::loadFromIfoFile(const QString& ifoFilePath,
                                bool isTreeDictionary)
{
    d->ifoFilePath = ifoFilePath;
    QFile ifoFile(ifoFilePath);
    QByteArray buffer = ifoFile.readAll();

    if (buffer.isEmpty())
        return false;

#define TREEDICT_MAGIC_DATA "StarDict's treedict ifo file\nversion=2.4.2\n"
#define DICT_MAGIC_DATA "StarDict's dict ifo file\nversion=2.4.2\n"

    const QByteArray magicData = isTreeDictionary ? TREEDICT_MAGIC_DATA : DICT_MAGIC_DATA;
    if (!buffer.startsWith(magicData))
        return false;

    QByteArray byteArray;
    int index;

    byteArray = buffer.mid(strlen(magicData) - 1);

    index = byteArray.indexOf("\nwordcount=");
    if (index == -1)
        return false;

    index += sizeof("\nwordcount=") - 1; 

    bool ok;
    d->wordCount = byteArray.mid(index, byteArray.indexOf('\n', index) - index).toLong(&ok, 10);

    if (isTreeDictionary)
    {
        index = byteArray.indexOf("\ntdxfilesize=");
        if (index == -1)
            return false;

        index += sizeof("\ntdxfilesize=");

        d->indexFileSize = byteArray.mid(index, byteArray.indexOf('\n', index) - index).toLong(&ok, 10);
    }
    else
    {
        index = byteArray.indexOf("\nidxfilesize=");
        if (index == -1)
            return false;

        index += sizeof("\nidxfilesize=");

        d->indexFileSize = byteArray.mid(index, byteArray.indexOf('\n', index) - index).toLong(&ok, 10);
    }

    // bookname
    index = byteArray.indexOf("\nbookname=");
    if (index == -1)
        return false;

    index += sizeof("\nbookname=");
    d->bookName = byteArray.indexOf('\n', index);

    // author
    index = byteArray.indexOf("\nauthor=");
    if (index == -1)
        return false;

    index += sizeof("\nauthor=");
    d->author = byteArray.indexOf('\n', index);

    // email
    index = byteArray.indexOf("\nemail=");
    if (index == -1)
        return false;

    index += sizeof("\nemail=");
    d->email = byteArray.indexOf('\n', index);

    // website
    index = byteArray.indexOf("\nwebsite=");
    if (index == -1)
        return false;

    index += sizeof("\nwebsite=");
    d->website = byteArray.indexOf('\n', index);

    // date
    index = byteArray.indexOf("\ndate=");
    if (index == -1)
        return false;

    index += sizeof("\ndate=");
    d->date = byteArray.indexOf('\n', index);

    // description
    index = byteArray.indexOf("\ndescription=");
    if (index == -1)
        return false;

    index += sizeof("\ndescription=");
    d->description = byteArray.indexOf('\n', index);

    // description
    index = byteArray.indexOf("\nsametypesequence=");
    if (index == -1)
        return false;

    index += sizeof("\nsametypesequence=");
    d->sameTypeSequence = byteArray.indexOf('\n', index);

    return true;
}

void
StarDictDictionaryInfo::setIfoFilePath(const QString& ifoFilePath)
{
    d->ifoFilePath = ifoFilePath;
}

QString
StarDictDictionaryInfo::ifoFilePath() const
{
    return d->ifoFilePath;
}

void
StarDictDictionaryInfo::setWordCount(quint32 wordCount)
{
    d->wordCount = wordCount;
}

quint32
StarDictDictionaryInfo::wordCount() const
{
    return d->wordCount;
}

void
StarDictDictionaryInfo::setBookName(const QString& bookName)
{
    d->bookName = bookName;
}

QString
StarDictDictionaryInfo::bookName() const
{
    return d->bookName;
}

void
StarDictDictionaryInfo::setAuthor(const QString& author)
{
    d->author = author;
}

QString
StarDictDictionaryInfo::author() const
{
    return d->author;
}

void
StarDictDictionaryInfo::setEmail(const QString& email)
{
    d->email = email;
}

QString
StarDictDictionaryInfo::email() const
{
    return d->email;
}

void
StarDictDictionaryInfo::setWebsite(const QString website)
{
    d->website = website;
}

QString
StarDictDictionaryInfo::website() const
{
    return d->website;
}

void
StarDictDictionaryInfo::setDateTime(const QString& dateTime)
{
    d->date = dateTime;
}

QString
StarDictDictionaryInfo::dateTime() const
{
    return d->date;
}

void
StarDictDictionaryInfo::setDescription(const QString& description)
{
    d->description = description;
}

QString
StarDictDictionaryInfo::description() const
{
    return d->description;
}

void
StarDictDictionaryInfo::setIndexFileSize(quint32 indexFileSize)
{
    d->indexFileSize = indexFileSize;
}

quint32
StarDictDictionaryInfo::indexFileSize() const
{
    return d->indexFileSize;
}

void
StarDictDictionaryInfo::setIndexOffsetBits(quint32 indexOffsetBits)
{
    d->indexOffsetBits = indexOffsetBits;
}

quint32
StarDictDictionaryInfo::indexOffsetBits() const
{
    return d->indexOffsetBits;
}

void
StarDictDictionaryInfo::setSameTypeSequence(const QString& sameTypeSequence)
{
    d->sameTypeSequence = sameTypeSequence;
}

QString
StarDictDictionaryInfo::sameTypeSequence() const
{
    return d->sameTypeSequence;
}

