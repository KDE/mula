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

#include "dictionaryplugin.h"

#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QVariant>

using namespace MulaCore;

DictionaryPlugin::DictionaryPlugin()
{
}

DictionaryPlugin::~DictionaryPlugin()
{
}

DictionaryPlugin::Features
DictionaryPlugin::features() const
{
    return None;
}

QStringList
DictionaryPlugin::findSimilarWords(const QString &dictionary, const QString &word)
{
    Q_UNUSED(dictionary)
    return QStringList(word);
}

int
DictionaryPlugin::execSettingsDialog(QWidget *parent)
{
    Q_UNUSED(parent);
    return 0;
}

QString
DictionaryPlugin::pluginDataPath() const
{
    QString path = QDir::homePath() + "/.config/mula/pluginsdata/" + name();

    if (!QDir::root().exists(path))
        QDir::root().mkpath(path);

    return path;
}
