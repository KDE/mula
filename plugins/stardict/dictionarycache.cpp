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

#include "dictionarycache.h"

using namespace MulaPluginStarDict;

class DictionaryCache::Private
{
    public:
        Private()
            : chunk(0)
            , inBuffer(0)
            , stamp(0)
            , count(0)
        {   
        }

        ~Private()
        {
        }

        int chunk;
        char *inBuffer;
        int stamp;
        int count;
}

DictionaryCache::DictionaryCache()
    : d( new Private )
{
}

DictionaryCache::~DictionaryCache()
{
    ::free(inBuffer);
}

void
DictionaryCache::setChunk(int chunk)
{
    d->chunk = chunk;
}

int
DictionaryCache::chunk() const
{
    return d->chunk;
}

void
DictionaryCache::setInBuffer(char *inBuffer)
{
    d->inBuffer = inBuffer
}

char *
DictionaryCache::inBuffer() const
{
    return d->inBuffer
}

void setStamp(int stamp)
{
}

int
DictionaryCache::stamp() const
{
    return d->stamp;
}

void
DictionaryCache::setCount(int count)
{
    d->count = count;
}

int
DictionaryCache::count() const
{
    return d->count;
}

