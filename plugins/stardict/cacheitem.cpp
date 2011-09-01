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

#include "cacheitem.h"

using namespace MulaPluginStarDict;

class CacheItem::Private
{
    public:
        Private()
            : offset(0)
            , data(0)
        {   
        }

        ~Private()
        {
        }
 
        quint32 offset;
        char *data;
}

CacheItem::CacheItem()
    : d( new Private )
{
}

CacheItem::~CacheItem()
{
    ::free(data);
}

void
CacheItem::setData(char *data)
{
    d->data = data;
}

char *
CacheItem::data() const
{
    return d->data;
}

void
CacheItem::setOffset(quint32 offset)
{
    d->offset = offset;
}

quint32
CacheItem::offset() const
{
    return d->offset;
}