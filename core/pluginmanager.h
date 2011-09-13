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

#ifndef MULA_CORE_PLUGINMANAGER_H
#define MULA_CORE_PLUGINMANAGER_H

#include "mula_core_export.h"

#include "singleton.h"
#include "dictionaryplugin.h"

#include <QtCore/QStringList>
#include <QtCore/QPair>
#include <QtCore/QHash>
#include <QtCore/QPluginLoader>

namespace MulaCore
{
    class MULA_CORE_EXPORT PluginManager : public MulaCore::Singleton<PluginManager>
    {
        Q_OBJECT
        MULA_SINGLETON( PluginManager )

        public:
            /**
             * Returns a list of available dictionary plugins.
             */
            QStringList availablePlugins() const;

            /**
             * Returns a list of loaded dictionary plugins.
             */
            QStringList loadedPlugins() const;

            /**
             * Sets the loaded plugins.
             * If plugin cannot be loaded it will not be added to
             * the list.
             */
            void setLoadedPlugins(const QStringList &loadedPlugins);

            /**
             * Returns pointer to plugin instance or 0 if not loaded.
             */
            DictionaryPlugin *plugin(const QString &plugin);

            /** 
             * Save the plugin settings.
             */
            void savePluginSettings();

        private:
            ~PluginManager();

            /** 
             * Load the plugin settings.
             */
            void loadPluginSettings();

            class Private;
            Private *const d;
    };
}

#endif // MULA_CORE_PLUGINMANAGER_H

