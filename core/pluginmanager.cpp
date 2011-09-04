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

#include "pluginmanager.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>

using namespace MULACore;

class PluginManager::Private
{
    public:
        PluginManagerPrivate()
        {
        }

        ~Private()
        {
        }

        QHash<QString, QPluginLoader*> plugins;
};

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
    loadSettings();
}

PluginManager::~PluginManager()
{
    saveSettings();
    foreach (QPluginLoader *loader, d->plugins)
    {
        delete loader->instance();
        delete loader;
    }
}

QStringList
PluginManager::availablePlugins() const
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

QStringList
PluginManager::loadedPlugins() const
{
    return d->plugins.keys();
}

void
PluginManager::setLoadedPlugins(const QStringList &loadedPlugins)
{
    for (QHash<QString, QPluginLoader*>::iterator i = d->plugins.begin(); i != d->plugins.end(); ++i)
    {
        delete (*i)->instance();
        delete *i;
    }

    d->plugins.clear();

    foreach (const QString& plugin, loadedPlugins)
    {
#ifdef Q_WS_X11
        QString pluginFileName = MULA_PLUGIN_DIR + "/lib" + plugin + ".so";

#elif defined Q_WS_WIN
        QString pluginFileName = MULA_PLUGIN_DIR + "/" + plugin + "0.dll";

#elif defined Q_WS_MAC
        // Follow Mac's bundle tree.
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

DictPlugin*
PluginManager::plugin(const QString &plugin)
{
    return d->plugins.contains(plugin) ? qobject_cast<DictPlugin*>(d->plugins[plugin]->instance()) : 0;
}

