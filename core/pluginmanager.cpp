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

#include "pluginmanager.h"

#include "directoryprovider.h"
#include "debughelper.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtCore/QDebug>
#include <QtCore/QSettings>

using namespace MulaCore;

MULA_DEFINE_SINGLETON( PluginManager )

class PluginManager::Private
{
    public:
        Private()
        {
        }

        ~Private()
        {
        }

        QHash<QString, QPluginLoader*> plugins;
};

PluginManager::PluginManager(QObject *parent)
    : MulaCore::Singleton< MulaCore::PluginManager >( parent )
    , d(new Private)
{
    loadPluginSettings();
}

PluginManager::~PluginManager()
{
    savePluginSettings();
    foreach (QPluginLoader *loader, d->plugins)
    {
        if (!loader->unload())
            qDebug() << "The plugin could not be unloaded:" << loader->errorString();

        delete loader;
    }
}

QStringList
PluginManager::availablePlugins() const
{
    DEBUG_FUNC_NAME
    QStringList pluginNameList;

    QStringList pluginDirectoryPaths = MulaCore::DirectoryProvider::instance()->pluginDirectoryPaths();

    DEBUG_TEXT2( "Number of plugin locations: %1", pluginDirectoryPaths.count() )
    foreach( const QString& pluginDirectoryPath, pluginDirectoryPaths )
    {
        QDir pluginDirectory( pluginDirectoryPath );
        DEBUG_TEXT( QString( "Looking for pluggable components in %1" ).arg( pluginDirectory.absolutePath() ) )

#ifdef Q_WS_X11
        //Only attempt to load our current version. This makes it possible to have different versions
        //of the plugins in the plugin dir.
        pluginDirectory.setNameFilters( QStringList() << QString( "*.so.%1.%2.%3" ).arg( MULA_VERSION_MAJOR ).arg( MULA_VERSION_MINOR ).arg( MULA_VERSION_PATCH ) );
#endif
        pluginDirectory.setFilter( QDir::AllEntries| QDir::NoDotAndDotDot );

        DEBUG_TEXT2( "Found %1 potential plugins. Attempting to load...", pluginDirectory.count() )
        foreach( const QString & fileName, pluginDirectory.entryList( QDir::Files ) )
        {
            // Do not attempt to load non-mula_plugin prefixed libraries
            if( !fileName.contains( "mula" ) )
                continue;

            // Don't attempt to load non-libraries
            if( !QLibrary::isLibrary( pluginDirectory.absoluteFilePath( fileName ) ) )
                continue;

            pluginNameList.append(pluginDirectory.absoluteFilePath( fileName ) );
        }
    }

    return pluginNameList;
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
        QString pluginFileName = "lib" + plugin + ".so";

#elif defined Q_WS_WIN
        QString pluginFileName = plugin + "0.dll";

#elif defined Q_WS_MAC
        // Follow Mac's bundle tree.
        QString pluginFileName = plugin;

#else
        qWarning() << "Function DictCore::setLoadedPlugins(const QStringList &loadedPlugins) is not available on this platform"
#endif

       QStringList pluginDirectoryPaths = MulaCore::DirectoryProvider::instance()->pluginDirectoryPaths();

        foreach( const QString& pluginDirectoryPath, pluginDirectoryPaths )
        {
            // Do not attempt to load non-libraries
            if( !QLibrary::isLibrary( pluginDirectoryPath + "/" + pluginFileName ) )
                continue;

            QPluginLoader *pluginLoader = new QPluginLoader(pluginDirectoryPath + "/" + pluginFileName);
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
}

DictionaryPlugin*
PluginManager::plugin(const QString &plugin)
{
    return d->plugins.contains(plugin) ? qobject_cast<DictionaryPlugin*>(d->plugins[plugin]->instance()) : 0;
}

void
PluginManager::savePluginSettings()
{
    QSettings settings;
    settings.setValue("PluginManager/loadedPlugins", loadedPlugins());
}

void
PluginManager::loadPluginSettings()
{
    QSettings settings;
    setLoadedPlugins(settings.value("PluginManager/loadedPlugins", availablePlugins()).toStringList());
}

#include "pluginmanager.moc"
