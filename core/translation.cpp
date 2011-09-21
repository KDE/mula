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

#include "translation.h"

using namespace MulaCore;

class Translation::Private
{
    public:
        Private()
        {   
        }

        ~Private()
        {
        }

        QString title;
        QString dictionaryName;
        QString translation;
};

Translation::Translation()
    : d(new Private)
{
}

Translation::Translation(const QString &title, const QString &dictionaryName,
                         const QString &translation)
    : d(new Private)
{
    d->title = title;
    d->dictionaryName = dictionaryName;
    d->translation = translation;
}

Translation::~Translation()
{
}

const QString&
Translation::title() const
{
    return d->title;
}

const QString&
Translation::dictionaryName() const
{
    return d->dictionaryName;
}

const QString&
Translation::translation() const
{
    return d->translation;
}

void
Translation::setTitle(const QString &title)
{
    d->title = title;
}

void
Translation::setDictionaryName(const QString &dictionaryName)
{
    d->dictionaryName = dictionaryName;
}
