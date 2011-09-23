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

using namespace MulaCore;

class DictionaryInfo::Private
{
    public:
        Private()
            : wordCount(0)
        {
        }

        ~Private()
        {
        }

        QString plugin;
        QString name;
        QString author;
        QString description;
        long wordCount;
};

DictionaryInfo::DictionaryInfo()
    : d(new Private)
{
}

DictionaryInfo::DictionaryInfo(const QString &plugin, const QString &name, const QString &author,
               const QString &description, long wordCount)
    : d(new Private)
{
    d->plugin = plugin;
    d->name = name;
    d->author = author;
    d->description = description;
    d->wordCount = wordCount;
}

DictionaryInfo::~DictionaryInfo()
{
}

const QString&
DictionaryInfo::plugin() const
{
    return d->plugin;
}

const QString&
DictionaryInfo::name() const
{
    return d->name;
}

const QString&
DictionaryInfo::author() const
{
    return d->author;
}

const QString&
DictionaryInfo::description() const
{
    return d->description;
}

long
DictionaryInfo::wordCount() const
{
    return d->wordCount;
}

void
DictionaryInfo::setPlugin(const QString &plugin)
{
    d->plugin = plugin;
}

void
DictionaryInfo::setName(const QString &name)
{
    d->name = name;
}

void
DictionaryInfo::setAuthor(const QString &author)
{
    d->author = author;
}

void
DictionaryInfo::setDescription(const QString &description)
{
    d->description = description;
}

void
DictionaryInfo::setWordCount(long wordCount)
{
    d->wordCount = wordCount;
}
