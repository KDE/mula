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

#include "dictionarydata.h"

using namespace MulaCore;

class DictionaryData::Private
{
    public:
        Private()
        {   
        }

        ~Private()
        {
        }

        QString plugin;
        QString name;
};

DictionaryData::DictionaryData(const QString &plugin, const QString &name)
    : d(new Private)
{
    d->plugin = plugin;
    d->name = name;
}

DictionaryData::DictionaryData()
    : d(new Private)
{
}

DictionaryData::~DictionaryData()
{
}

const QString&
DictionaryData::plugin() const
{   
    return d->plugin;
}   

const QString&
DictionaryData::name() const
{   
    return d->name;
}   

void
DictionaryData::setPlugin(const QString &plugin)
{   
    d->plugin = plugin;
}   

void
DictionaryData::setName(const QString &name)
{   
    d->name = name;
}   

bool
DictionaryData::operator == (const DictionaryData &dictionaryData)
{   
    return d->name == dictionaryData.name() && d->plugin == dictionaryData.plugin();
} 
