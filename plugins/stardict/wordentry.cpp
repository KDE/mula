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

#include "wordentry.h"

using namespace MulaPluginStarDict;

class WordEntry::Private : public QSharedData
{
    public:
        Private()
            : dataOffset(0)
            , dataSize(0)
        {   
        }

        ~Private()
        {
        }
 
        QByteArray data;
        quint32 dataOffset;
        quint32 dataSize;
};

WordEntry::WordEntry()
    : d(new Private)
{
}

WordEntry::WordEntry(const WordEntry &other):
    d(other.d)
{
}

WordEntry::~WordEntry()
{
}

WordEntry&
WordEntry::operator=(const WordEntry &other)
{
    d = other.d;
    return *this;
}

void
WordEntry::setData(QByteArray data)
{
    d->data = data;
}

QByteArray
WordEntry::data() const
{
    return d->data;
}

void
WordEntry::setDataOffset(quint32 dataOffset)
{
    d->dataOffset = dataOffset;
}

quint32
WordEntry::dataOffset() const
{
    return d->dataOffset;
}

void
WordEntry::setDataSize(quint32 dataSize)
{
    d->dataSize = dataSize;
}

quint32
WordEntry::dataSize() const
{
    return d->dataSize;
}
