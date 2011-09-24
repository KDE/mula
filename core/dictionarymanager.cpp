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

#include "dictionarymanager.h"

#include "pluginmanager.h"

#include <QtCore/QFileInfoList>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QPluginLoader>

using namespace MulaCore;

MULA_DEFINE_SINGLETON( DictionaryManager )

class DictionaryManager::Private
{
    public:
        Private()
        {
        }

        ~Private()
        {
        }

        QList<DictionaryDataItem> loadedDictionaryList;
};

DictionaryManager::DictionaryManager(QObject *parent)
    : MulaCore::Singleton< MulaCore::DictionaryManager >( parent )
    , d(new Private)
{
    loadDictionarySettings();
}

DictionaryManager::~DictionaryManager()
{
    saveDictionarySettings();
}

bool
DictionaryManager::isTranslatable(const QString &word)
{
    foreach (const DictionaryDataItem& dictionary, d->loadedDictionaryList)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(dictionary.plugin());
        if (!dictionaryPlugin)
            continue;

        if (dictionaryPlugin->isTranslatable(dictionary.name(), word))
            return true;
    }

    return false;
}

QString
DictionaryManager::translate(const QString &word)
{
    QString simplifiedWord = word.simplified();
    QString translatedWord;

    foreach (const DictionaryDataItem& dictionary, d->loadedDictionaryList)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(dictionary.plugin());
        if (!dictionaryPlugin)
            continue;

        if (!dictionaryPlugin->isTranslatable(dictionary.name(), simplifiedWord))
            continue;

        MulaCore::Translation translation = dictionaryPlugin->translate(dictionary.name(), simplifiedWord);
        translatedWord.append(QString("<p>\n")
            + "<font class=\"dict_name\">" + translation.dictionaryName() + "</font><br>\n"
            + "<font class=\"title\">" + translation.title() + "</font><br>\n"
            + translation.translation() + "</p>\n");
    }

    return translatedWord;
}

QStringList
DictionaryManager::findSimilarWords(const QString &word)
{
    QString simplifiedWord = word.simplified();
    QStringList similarWords;

    foreach (const DictionaryDataItem& dictionary, d->loadedDictionaryList)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(dictionary.plugin());
        if (!dictionaryPlugin)
            continue;

        if (!dictionaryPlugin->features().testFlag(DictionaryPlugin::SearchSimilar))
            continue;

        QStringList similarWords = dictionaryPlugin->findSimilarWords(dictionary.name(), simplifiedWord);
        foreach (const QString& similarWord, similarWords)
        {
            if (!similarWords.contains(similarWord, Qt::CaseSensitive))
                similarWords.append(similarWord);
        }
    }

    return similarWords;
}

QList<DictionaryDataItem>
DictionaryManager::availableDictionaryList() const
{
    QList<DictionaryDataItem> availableDictionaryList;

    // for (QHash<QString, QPluginLoader*>::const_iterator i = m_plugins.begin(); i != m_plugins.end(); ++i)
    // {
        // DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>((*i)->instance());
        // QStringList dictionaries = plugin->availableDictionaryList();
        // foreach (const QString& dictionary, dictionaries)
            // availableDictionaryList.append(DictionaryDataItem(i.key(), dictionary));
    // }

    return availableDictionaryList;
}

void
DictionaryManager::setLoadedDictionaryList(const QList<DictionaryDataItem> &loadedDictionaryList)
{
    QHash<QString, QStringList> dictionaries;
    foreach (const DictionaryDataItem& dictionary, loadedDictionaryList)
        dictionaries[dictionary.plugin()] = QStringList() << dictionary.name();

    for (QHash<QString, QStringList>::const_iterator i = dictionaries.begin(); i != dictionaries.end(); ++i)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(i.key());
        if (!dictionaryPlugin)
            continue;

        dictionaryPlugin->setLoadedDictionaryList(*i);
        dictionaries[i.key()] = dictionaryPlugin->loadedDictionaryList();
    }

    d->loadedDictionaryList.clear();
    foreach (const DictionaryDataItem& dictionary, loadedDictionaryList)
    {
        if (dictionaries.contains(dictionary.plugin()) && dictionaries[dictionary.plugin()].contains(dictionary.name()))
            d->loadedDictionaryList.append(dictionary);
    }
}

void
DictionaryManager::saveDictionarySettings()
{
    QStringList rawDictionaryList;

    foreach (const DictionaryDataItem& dictionary, d->loadedDictionaryList)
    {
        rawDictionaryList.append(dictionary.plugin());
        rawDictionaryList.append(dictionary.name());
    }

    QSettings settings;
    settings.setValue("DictionaryManager/loadedDictionaryList", rawDictionaryList);
}

void
DictionaryManager::loadDictionarySettings()
{
    QSettings settings;
    QStringList rawDictionaryList = settings.value("DictionaryManager/loadedDictionaryList").toStringList();

    if (rawDictionaryList.isEmpty())
    {
        setLoadedDictionaryList(availableDictionaryList());
    }
    else
    {
        QList<DictionaryDataItem> dictionaries;
        for (QStringList::const_iterator i = rawDictionaryList.begin(); i != rawDictionaryList.end(); i += 2)
            dictionaries.append(DictionaryDataItem(*i, *(i + 1)));

        setLoadedDictionaryList(dictionaries);
    }
}

void
DictionaryManager::reloadDictionaryList()
{
    QList<DictionaryDataItem> loadedDictionaryList;
    // for (QHash<QString, QPluginLoader*>::const_iterator i = d->plugins.begin(); i != d->plugins.end(); ++i)
    // {
        // DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>((*i)->instance());
        // plugin->setLoadedDictionaryList(plugin->loadedDictionaryList());

        // foreach(const QString& dictionaryName, plugin->loadedDictionaryList())
            // loadedDictionaryList.append(DictionaryDataItem(i.key(), dictionaryName));
    // }

    QList<DictionaryDataItem> oldDictionaryList = d->loadedDictionaryList;
    d->loadedDictionaryList.clear();
 
    foreach (const DictionaryDataItem& dictionary, oldDictionaryList)
    {
        if (loadedDictionaryList.contains(dictionary))
            d->loadedDictionaryList.append(dictionary);
    }
}

#include "dictionarymanager.moc"
