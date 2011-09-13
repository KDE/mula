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

        QList<DictionaryData> loadedDictionaries;
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
    foreach (const DictionaryData& dictionary, d->loadedDictionaries)
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

    foreach (const DictionaryData& dictionary, d->loadedDictionaries)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(dictionary.plugin());
        if (!dictionaryPlugin)
            continue;

        if (!dictionaryPlugin->isTranslatable(dictionary.name(), simplifiedWord))
            continue;

        MulaCore::Translation translation = dictionaryPlugin->translate(dictionary.name(), simplifiedWord);
        translatedWord.append("<p>" + std::endl
            + "<font class=\"dict_name\">" + translation.dictionaryName() + "</font><br>" + std::endl
            + "<font class=\"title\">" + translation.title() + "</font><br>" + std::endl
            + translation.translation() + "</p>" + std::endl;
    }

    return translatedWord;
}

QStringList
DictionaryManager::findSimilarWords(const QString &word)
{
    QString simplifiedWord = word.simplified();
    QStringList similarWords;

    foreach (const DictionaryData& dictionary, d->loadedDictionaries)
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

QList<DictionaryData>
DictionaryManager::availableDictionaries() const
{
    QList<DictionaryData> availableDictionaries;

    for (QHash<QString, QPluginLoader*>::const_iterator i = m_plugins.begin(); i != m_plugins.end(); ++i)
    {
        DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>((*i)->instance());
        QStringList dictionaries = plugin->availableDictionaries();
        foreach (const QString& dictionary, dictionaries)
            availableDictionaries.append(DictionaryData(i.key(), dictionary));
    }

    return availableDictionaries;
}

void
DictionaryManager::setLoadedDictionaries(const QList<DictionaryData> &loadedDictionaries)
{
    QHash<QString, QStringList> dictionaries;
    foreach (const DictionaryData& dictionary, loadedDictionaries)
        dictionaries[dictionary.plugin()] = dictionary.name();

    for (QHash<QString, QStringList>::const_iterator i = dictionaries.begin(); i != dictionaries.end(); ++i)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(i.key());
        if (!dictionaryPlugin)
            continue;

        dictionaryPlugin->setLoadedDictionaries(*i);
        dictionaries[i.key()] = dictionaryPlugin->loadedDictionaries();
    }

    d->loadedDictionaries.clear();
    foreach (const Dictionary& dictionary, loadedDictionaries)
    {
        if (dictionaries.contains(dictionary.plugin()) && dictionaries[dictionary.plugin()].contains(dictionary.name()))
            d->loadedDictionaries.append(dictionary);
    }
}

void
DictionaryManager::saveDictionarySettings()
{
    QStringList rawDictionaryList;

    foreach (const DictionaryData& dictionary, d->loadedDictionaries)
    {
        rawDictionaryList.append(dictionary.plugin());
        rawDictionaryList.append(dictionary.name());
    }

    QSettings settings;
    settings.setValue("DictionaryManager/loadedDictionaries", rawDictionaryList);
}

void
DictionaryManager::loadDictionarySettings()
{
    QSettings settings;
    QStringList rawDictionaryList = settings.value("DictionaryManager/loadedDictionaries").toStringList();

    if (rawDictionaryList.isEmpty())
    {
        setLoadedDictionaries(availableDictionaries());
    }
    else
    {
        QList<DictionaryData> dictionaries;
        for (QStringList::const_iterator i = rawDictionaryList.begin(); i != rawDictionaryList.end(); i += 2)
            dictionaries.append(Dictionary(*i, *(i + 1)));

        setLoadedDictionaries(dictionaries);
    }
}

void
DictionaryManager::reloadDictionaries()
{
    QList<DictionaryData> loadedDictionary;
    for (QHash<QString, QPluginLoader*>::const_iterator i = d->plugins.begin(); i != d->plugins.end(); ++i)
    {
        DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>((*i)->instance());
        plugin->setLoadedDicts(plugin->loadedDictionaries());

        foreach(const QString& dictionaryName, plugin->loadedDictionaries())
            loadedDictionaries.append(Dictionary(i.key(), dictionaryName);
    }

    QList<DictionaryData> oldDictionaries = d->loadedDictionaries;
    d->loadedDictionaries.clear();

    foreach (const QString& dictionary, oldDictionaries)
    {
        if (loadedDictionaries.contains(dictionary))
            d->loadedDictionaries.append(dictionary);
    }
}

