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

#include "dictionarydataitem.h"

using namespace MulaCore;

class DictionaryDataItem::Private
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

DictionaryDataItem::DictionaryDataItem(const QString &plugin, const QString &name)
    : d(new Private)
{
    d->plugin = plugin;
    d->name = name;
}

DictionaryDataItem::DictionaryDataItem()
    : d(new Private)
{
}

DictionaryDataItem::~DictionaryDataItem()
{
}

const QString&
DictionaryDataItem::plugin() const
{   
    return d->plugin;
}   

const QString&
DictionaryDataItem::name() const
{   
    return d->name;
}   

void
DictionaryDataItem::setPlugin(const QString &plugin)
{   
    d->plugin = plugin;
}   

void
DictionaryDataItem::setName(const QString &name)
{   
    d->name = name;
}   

bool
DictionaryDataItem::operator == (const DictionaryDataItem &dictionaryDataItem)
{   
    return d->name == dictionaryDataItem.name() && d->plugin == dictionaryDataItem.plugin();
} 
