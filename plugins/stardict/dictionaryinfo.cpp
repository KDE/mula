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

#include "dictionaryinfo.h"

using namespace MulaPluginStarDict;

class DictionaryInfo::Private
{
    public:
        Private()
            : wordcount(0)
            , indexFileSize(0)
            , indexOffsetBits(0)
        {   
        }

        ~Private()
        {
        }
 
        QString ifoFileName;
        quint32 wordcount;
        QString bookname;
        QString author;
        QString email;
        QString website;
        QDateTime date;
        QString description;
        quint32 indexFileSize;
        quint32 indexOffsetBits;
        QString sameTypeSequence;
}

DictionaryInfo::DictionaryInfo()
    : d(new Private)
{
}

DictionaryInfo::~DictionaryInfo()
{
}

bool
DictionaryInfo::loadFromIfoFile(const QString& ifoFileName,
                                bool isTreeDictionary)
{
    d->ifoFileName = ifoFileName;
    QFile ifoFile(ifoFileName);
    QByteArray buffer = ifoFile.readAll();
    if (buffer.isEmpty())
        return false;

#define TREEDICT_MAGIC_DATA "StarDict's treedict ifo file\nversion=2.4.2\n"
#define DICT_MAGIC_DATA "StarDict's dict ifo file\nversion=2.4.2\n"

    const QString magicData = isTreeDictionary ? TREEDICT_MAGIC_DATA : DICT_MAGIC_DATA;
    if (!buffer.startsWith(magic_data))
        return false;

    QByteArray byteArray;
    int index;

    byteArray = buffer.mid(strlen(magic_data) - 1);

    index = byteArray.indexOf("\nwordcount=");
    if (index == -1)
        return false;

    index += sizeof("\nwordcount=") - 1; 

    bool ok;
    d->wordCount = byteArray.mid(index, p1.indexOf('\n', index) - index).toLong(&ok, 10);

    if (d->isTreeDictionary)
    {
        index = byteArray.indexOf("\ntdxfilesize=");
        if (index == -1)
            return false;

        index += sizeof("\ntdxfilesize=");

        d->indexFileSize = byteArray.mid(index, p1.indexOf('\n', index) - index).toLong(&ok, 10);
    }
    else
    {
        index = byteArray.indexOf("\nidxfilesize=");
        if (index == -1)
            return false;

        index += sizeof("\nidxfilesize=");

        d->indexFileSize = byteArray.mid(index, p1.indexOf('\n', index) - index).toLong(&ok, 10);
    }

    // bookname
    index = byteArray.indexOf("\nbookname=");
    if (index == -1)
        return false;

    index += sizeof("\nbookname=");
    d->bookname.assign(index, p1.indexOf('\n', index));

    // author
    index = byteArray.indexOf("\nauthor=");
    if (index == -1)
        return false;

    index += sizeof("\nauthor=");
    d->author.assign(index, p1.indexOf('\n', index));

    // email
    index = byteArray.indexOf("\nemail=");
    if (index == -1)
        return false;

    index += sizeof("\nemail=");
    d->email.assign(index, p1.indexOf('\n', index));

    // website
    index = byteArray.indexOf("\nwebsite=");
    if (index == -1)
        return false;

    index += sizeof("\nwebsite=");
    d->website.assign(index, p1.indexOf('\n', index));

    // date
    index = byteArray.indexOf("\ndate=");
    if (index == -1)
        return false;

    index += sizeof("\ndate=");
    d->date.assign(index, p1.indexOf('\n', index));

    // description
    index = byteArray.indexOf("\ndescription=");
    if (index == -1)
        return false;

    index += sizeof("\ndescription=");
    d->description.assign(index, p1.indexOf('\n', index));

    // description
    index = byteArray.indexOf("\nsametypesequence=");
    if (index == -1)
        return false;

    index += sizeof("\nsametypesequence=");
    d->sameTypeSequence.assign(index, p1.indexOf('\n', index));

    return true;
}

void
DictionaryInfo::setIfoFileName(const QString& ifoFileName)
{
    d->ifoFileName = ifoFileName;
}

QString
DictionaryInfo::ifoFileName() const
{
    return d->ifoFileName;
}

void
DictionaryInfo::setWordCount(quint32 wordCount)
{
    d->wordcount = wordCount;
}

quint32
DictionaryInfo::wordCount() const
{
    return d->wordcount;
}

void
DictionaryInfo::setBookName(const QString& bookname)
{
    d->bookname = bookname;
}

QString
DictionaryInfo::bookName() const
{
    return d->bookname;
}

void
DictionaryInfo::setAuthor(const QString& author)
{
    d->author = author;
}

QString
DictionaryInfo::author() const
{
    return d->author;
}

void
DictionaryInfo::setEmail(const QString& email)
{
    d->email = email;
}

QString
DictionaryInfo::email() const
{
    return d->email;
}

void
DictionaryInfo::setWebsite(const QString website)
{
    d->website = website;
}

QString
DictionaryInfo::website() const
{
    return d->website;
}

void
DictionaryInfo::setDate(const QDateTime& dateTime)
{
    d->date = dateTime;
}

QDateTime
DictionaryFile::dateTime() const
{
    return d->date;
}

void
DictionaryInfo::setDescription(const QString& description)
{
    d->description = description;
}

QString
DictionaryInfo::description() const
{
    return d->description;
}

void
DictionaryInfo::setIndexFileSize(quint32 indexFileSize)
{
    d->indexFileSize = indexFileSize;
}

quint32
DictionaryInfo::indexFileSize() const
{
    return d->indexFileSize;
}

void
DictionaryInfo::setIndexOffsetBits(quint32 indexOffsetBits)
{
    d->indexOffsetBits = indexOffsetBits;
}

quint32
DictionaryInfo::indexOffsetBits() const
{
    d->indexOffsetBits;
}

void
DictionaryInfo::setSameTypeSequence(const QString& sameTypeSequence)
{
    d->sameTypeSequence = sameTypeSequence;
}

QString
DictionaryInfo::sameTypeSequence() const
{
    return d->sameTypeSequence;
}

