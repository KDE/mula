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

        QMultiHash<QString, QString> loadedDictionaryList;
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
    for (QMultiHash<QString, QString>::const_iterator i = d->loadedDictionaryList.begin(); i != d->loadedDictionaryList.end(); ++i)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(i.key());
        if (!dictionaryPlugin)
            continue;

        if (dictionaryPlugin->isTranslatable(i.value(), word))
            return true;
    }

    return false;
}

QString
DictionaryManager::translate(const QString &word)
{
    QString simplifiedWord = word.simplified();
    QString translatedWord;

    for (QMultiHash<QString, QString>::const_iterator i = d->loadedDictionaryList.begin(); i != d->loadedDictionaryList.end(); ++i)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(i.key());
        if (!dictionaryPlugin)
            continue;

        if (!dictionaryPlugin->isTranslatable(i.key(), simplifiedWord))
            continue;

        MulaCore::Translation translation = dictionaryPlugin->translate(i.key(), simplifiedWord);
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

    for (QMultiHash<QString, QString>::const_iterator i = d->loadedDictionaryList.begin(); i != d->loadedDictionaryList.end(); ++i)
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(i.key());
        if (!dictionaryPlugin)
            continue;

        if (!dictionaryPlugin->features().testFlag(DictionaryPlugin::SearchSimilar))
            continue;

        QStringList similarWords = dictionaryPlugin->findSimilarWords(i.value(), simplifiedWord);
        foreach (const QString& similarWord, similarWords)
        {
            if (!similarWords.contains(similarWord, Qt::CaseSensitive))
                similarWords.append(similarWord);
        }
    }

    return similarWords;
}

QMultiHash<QString, QString>
DictionaryManager::availableDictionaryList() const
{
    QMultiHash<QString, QString> availableDictionaryList;

    foreach (const QString& pluginName, MulaCore::PluginManager::instance()->availablePlugins())
    {
        DictionaryPlugin *dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(pluginName);
        QStringList dictionaries = dictionaryPlugin->availableDictionaryList();
        foreach (const QString& dictionaryName, dictionaries)
            availableDictionaryList.insert(pluginName, dictionaryName);
    }

    return availableDictionaryList;
}

void
DictionaryManager::setLoadedDictionaryList(const QMultiHash<QString, QString> &loadedDictionaryList)
{
    QMultiHash<QString, QString> dictionaries = loadedDictionaryList;

    foreach (const QString& pluginName, dictionaries.keys())
    {
        MulaCore::DictionaryPlugin* dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(pluginName);
        if (!dictionaryPlugin)
            continue;

        dictionaryPlugin->setLoadedDictionaryList(dictionaries.values());
        foreach (const QString& dictionaryName, dictionaryPlugin->loadedDictionaryList())
                dictionaries.insert(pluginName, dictionaryName);
    }

    d->loadedDictionaryList.clear();
    for (QMultiHash<QString, QString>::const_iterator i = loadedDictionaryList.begin(); i != loadedDictionaryList.end(); ++i)
    {
        if (dictionaries.keys().contains(i.key()) && dictionaries.value(i.key()).contains(i.value()))
            d->loadedDictionaryList.insert(i.key(), i.value());
    }
}

void
DictionaryManager::saveDictionarySettings()
{
    QStringList rawDictionaryList;

    for (QMultiHash<QString, QString>::const_iterator i = d->loadedDictionaryList.begin(); i != d->loadedDictionaryList.end(); ++i)
    {
        rawDictionaryList.append(i.key());
        rawDictionaryList.append(i.value());
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
        QMultiHash<QString, QString> dictionaries;
        for (QStringList::const_iterator i = rawDictionaryList.begin(); i != rawDictionaryList.end(); i += 2)
            dictionaries.insert(*i, *(i + 1));

        setLoadedDictionaryList(dictionaries);
    }
}

void
DictionaryManager::reloadDictionaryList()
{
    QMultiHash<QString, QString> loadedDictionaryList;
    foreach (const QString& pluginName, MulaCore::PluginManager::instance()->availablePlugins())
    {
        DictionaryPlugin *dictionaryPlugin = MulaCore::PluginManager::instance()->plugin(pluginName);
        dictionaryPlugin->setLoadedDictionaryList(dictionaryPlugin->loadedDictionaryList());
        QStringList dictionaries = dictionaryPlugin->availableDictionaryList();
        foreach (const QString& dictionaryName, dictionaryPlugin->loadedDictionaryList())
            loadedDictionaryList.insert(pluginName, dictionaryName);
    }

    QMultiHash<QString, QString> oldDictionaryList = d->loadedDictionaryList;
    d->loadedDictionaryList.clear();
 
    for (QMultiHash<QString, QString>::const_iterator i = oldDictionaryList.begin(); i != oldDictionaryList.end(); ++i)
    {
        if (loadedDictionaryList.contains(i.key(), i.value()))
            d->loadedDictionaryList.insert(i.key(), i.value());
    }
}

#include "dictionarymanager.moc"
