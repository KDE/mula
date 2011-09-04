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

#include "idictionaryplugin.h"

using namespace MULAPlugin;


DictionaryData(const QString &plugin, const QString &name, const QString &author = QString(), 
        const QString &description = QString(), long wordsCount = -1L)
    : m_plugin(plugin)
    , m_name(name)
    , m_author(author)
    , m_description(description)
    , m_wordsCount(wordsCount)
{
}

const QString&
DictionaryData::plugin() const
{ 
    return m_plugin; 
}

const QString&
DictionaryData::name() const
{
    return m_name;
}

const QString&
DictionaryData::author() const
{ 
    return m_author;
}

const QString&
DictionaryData::description() const
{
    return m_description;
}

long
DictionaryData::wordsCount() const
{
    return m_wordsCount;
}

void
DictionaryData::setPlugin(const QString &plugin)
{
    m_plugin = plugin;
}

void
DictionaryData::setName(const QString &name)
{
    m_name = name;
}

void
DictionaryData::setAuthor(const QString &author)
{
    m_author = author;
}

void
DictionaryData::setDescription(const QString &description)
{
    m_description = description;
}

void
DictionaryData::setWordsCount(long wordsCount)
{
    m_wordsCount = wordsCount;
}

Translation::Translation(const QString &title, const QString &dictName,
                         const QString &translation);
    : m_title(title)
    , m_dictName(dictName)
    , m_translation(translation)
{
}

const QString&
Translation::title() const
{
    return m_title;
}

const QString&
Translation::dictName() const
{
    return m_dictName;
}

const QString&
Translation::translation() const
{
    return m_translation;
}

void
Translation::setTitle(const QString &title)
{
    m_title = title;
}

void
Translation::setDictName(const QString &dictName)
{
    m_dictName = dictName;
}

IDictionaryPlugin::IDictionaryPlugin()
{
}

IDictionaryPlugin::~IDictionaryPlugin()
{
}

Features
IDictionaryPlugin::features() const
{
    return None;
}

QStringList
IDictionaryPlugin::findSimilarWords(const QString &dict, const QString &word)
{
    Q_UNUSED(dict)
    return QStringList(word);
}

QVariant
IDictionaryPlugin::resource(int type, const QUrl& name)
{
    Q_UNUSED(type)
    Q_UNUSED(name)
    return QVariant();
}

int
IDictionaryPlugin::execSettingsDialog(QWidget *parent = 0)
{
    Q_UNUSED(parent);
    return 0;
}

QString
IDictionaryPlugin::workPath() const
{
    QString path = QDir::homePath() + "/.config/mula/pluginsdata/" + name();

    if (!QDir::root().exists(path))
        QDir::root().mkpath(path);

    return path;
}

#include "idictionaryplugin.moc"
