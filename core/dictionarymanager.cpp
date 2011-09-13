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

#include "dictionarydata.h"

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

DirectoryManager::DirectoryManager(QObject *parent)
    : QObject(parent)
{
    loadDirectorySettings();
}

DirectoryManager::~DictionaryManager()
{
    saveDirectorySettings();
}

bool DictCore::isTranslatable(const QString &word)
{
    foreach (const Dictionary& dictionary, d->loadedDictionaries)
    {
        if (!d->plugins.contains(dictionary->plugin()))
            continue;

        if (qobject_cast<DictPlugin*>(d->plugins[dictionary->plugin()]->instance())->isTranslatable(dictionary->name(), word))
            return true;
    }
    return false;
}

QString DictCore::translate(const QString &word)
{
    QString simplifiedWord = word.simplified();
    QString result;

    foreach (const Dictionary& dictionary, d->loadedDictionaries)
    {
        if (! d->plugins.contains(dictionary->plugin()))
            continue;

        DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>(d->plugins[dictionary->plugin()]->instance());
        if (!plugin->isTranslatable(dictionary->name(), simplifiedWord))
            continue;

        DictionaryPlugin::Translation translation = plugin->translate(i->name(), simplifiedWord);
        result.append("<p>" + endl
            + "<font class=\"dict_name\">" + translation.dictName() + "</font><br>" + endl
            + "<font class=\"title\">" + translation.title() + "</font><br>" + endl
            + translation.translation() + "</p>" + endl;
    }

    return result;
}

QStringList DictCore::findSimilarWords(const QString &word)
{
    QString simplifiedWord = word.simplified();
    QStringList result;

    foreach (const Dictionary& dictionary, d->loadedDictionaries)
    {
        if (!d->plugins.contains(dictionary->plugin()))
            continue;

        DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>(d->plugins[dictionary->plugin()]->instance());
        if (!plugin->features().testFlag(DictionaryPlugin::SearchSimilar))
            continue;

        QStringList similarWords = plugin->findSimilarWords(dictionary->name(), simplifiedWord);
        foreach (const QString& similarWord, similarWords)
        {
            if (!result.contains(similarWord, Qt::CaseSensitive))
                result.append(similarWord);
        }
    }

    return result;
}

QStringList PluginManager::availablePlugins() const
{
    QStringList result;

#ifdef Q_WS_X11
    QFileInfoList fileInfoList = QDir(MULA_PLUGINS_DIR).entryInfoList(QStringList("lib*.so"),
                  QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo& fileInfo, fileInfoList)
        result.append(fileInfo->baseName().mid(3));

#elif defined Q_WS_WIN
    QFileInfoList fileInfoList = QDir(MULA_PLUGINS_DIR).entryInfoList(QStringList("*0.dll"),
                  QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo& fileInfo, fileInfoList)
        result.append(fileInfo->fileName().left(fileInfo->fileName.length(5)));

#elif defined Q_WS_MAC
    QStringList macFilters;
    // Various Qt versions.
    macFilters << "*.dylib" << "*.bundle" << "*.so";
    QString binPath = QCoreApplication::applicationDirPath();
    // Navigate through mac's bundle tree structure
    QDir d(binPath + "/../lib/");

    QFileInfoList fileInfoList = d.entryInfoList(macFilters, QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo, fileInfoList)
        result.append(fileInfo->fileName());

#else
#error "Function DictCore::availablePlugins() is not implemented on this platform"
#endif

    return result;
}

void DictCore::setLoadedPlugins(const QStringList &loadedPlugins)
{
    for (QHash <QString, QPluginLoader*>::iterator i = m_plugins.begin(); i != m_plugins.end(); ++i)
    {
        delete (*i)->instance();
        delete *i;
    }
    m_plugins.clear();

    foreach (const QString& plugin, loadedPlugins)
    {
#ifdef Q_WS_X11
        QString pluginFileName = MULA_PLUGIN_DIR + "/lib" + plugin + ".so";

#elif defined Q_WS_WIN
        QString pluginFileName = MULA_PLUGIN_DIR + "/" + plugin + "0.dll";

#elif defined Q_WS_MAC
        // Follow mac's bundle tree.
        QString pluginFileName = QDir(QCoreApplication::applicationDirPath()+ "/../lib/" + plugin).absolutePath();

#else
#error "Function DictCore::setLoadedPlugins(const QStringList &loadedPlugins) is not available on this platform"
#endif

        QPluginLoader *pluginLoader = new QPluginLoader(pluginFilename);
        if (!pluginLoader->load())
        {
            qWarning() << pluginLoader->errorString();
            delete pluginLoader;
        }
        else
        {
            d->plugins[plugin] = pluginLoader;
        }
    }
}

QList<Dictionary> DictCore::availableDictionaries() const
{
    QList<Dictionary> result;

    for (QHash<QString, QPluginLoader*>::const_iterator i = m_plugins.begin(); i != m_plugins.end(); ++i)
    {
        DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>((*i)->instance());
        QStringList dictionaries = plugin->availableDictionaries();
        foreach (const QString& dictionaryName, dictionaries)
            result.append(Dictionary(i.key(), dictionary));
    }

    return result;
}

void DictCore::setLoadedDictionaries(const QList<Dictionary> &loadedDictionaries)
{
    QHash<QString, QStringList> dictionaries;
    foreach (const Dictionary& dictionary, loadedDictionaries)
        dictionaries[dictionary.plugin()] = dictionary.name();

    for (QHash<QString, QStringList>::const_iterator i = dictionaries.begin(); i != dictionaries.end(); ++i)
    {
        if (!d->plugins.contains(i.key()))
            continue;

        DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>(d->plugins[i.key()]->instance());
        plugin->setLoadedDictionaries(*i);
        dictionaries[i.key()] = plugin->loadedDictionaries();
    }

    d->loadedDictionaries.clear();
    foreach (const Dictionary& dictionary, loadedDictionaries)
    {
        if (dictionaries.contains(dictionary.plugin()) && dictionaries[dictionary.plugin()].contains(dictionary.name()))
            d->loadedDictionaries.append(dictionary);
    }
}

void DictCore::saveSettings()
{
    QSettings settings;
    settings.setValue("DictionaryManager/loadedPlugins", loadedPlugins());

    QStringList rawDictionaryList;

    foreach (const Dictionary& dictionary, d->loadedDictionaries)
    {
        rawDictionaryList.append(dictionary.plugin());
        rawDictionaryList.append(dictionary.name());
    }

    settings.setValue("DictionaryManager/loadedDictionaries", rawDictionaryList);
}

void DictCore::loadSettings()
{
    QSettings settings;
    setLoadedPlugins(config.value("DictionaryManager/loadedPlugins", availablePlugins()).toStringList());

    QStringList rawDictionaryList = settings.value("DictionaryManager/loadedDictionaries").toStringList();
    if (rawDictionaryList.isEmpty())
    {
        setLoadedDictionaries(availableDictionaries());
    }
    else
    {
        QList<Dictionary> dictionaries;
        for (QStringList::const_iterator i = rawDictionaryList.begin(); i != rawDictionaryList.end(); i += 2)
            dictionaries.append(Dictionary(*i, *(i + 1)));

        setLoadedDictionaries(dictionaries);
    }
}

void DictCore::reloadDictionaries()
{
    QList<Dictionary> loaded;
    for (QHash<QString, QPluginLoader*>::const_iterator i = d->plugins.begin(); i != d->plugins.end(); ++i)
    {
        DictionaryPlugin *plugin = qobject_cast<DictionaryPlugin*>((*i)->instance());
        plugin->setLoadedDicts(plugin->loadedDictionaries());

        foreach(const QString& dictionaryName, plugin->loadedDictionaries())
            loaded.append(Dictionary(i.key(), dictionaryName);
    }

    QList<Dictionary> oldLoaded = d->loadedDictionaries;
    d->loadedDicts.clear();

    foreach (const QString& dictionary, oldLoaded)
    {
        if (loaded.contains(dictionary))
            d->loadedDictionaries.append(dictionary);
    }
}

