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

#ifndef MULA_CORE_DICTIONARYMANAGER_H
#define MULA_CORE_DICTIONARYMANAGER_H

#include <plugins/dictplugin.h>

#include <QtCore/QStringList>
#include <QtCore/QPair>
#include <QtCore/QHash>
#include <QtCore/QPluginLoader>

namespace QStarDict
{

/**
 * The DictCore is a base dictionary class.
 */
class DictCore: public QObject
{
    Q_OBJECT

    public:
        /**
         * This class represents a dictionary.
         */
        class Dictionary
        {
            public:
                Dictionary(const QString &plugin, const QString &name)
                    : m_plugin(plugin),
                      m_name(name)
                {  }
                Dictionary()
                { }
                
                const QString &plugin() const
                { return m_plugin; }
                const QString &name() const
                { return m_name; }
                void setPlugin(const QString &plugin)
                { m_plugin = plugin; }
                void setName(const QString &name)
                { m_name = name; }
                bool operator == (const Dictionary &dict)
                { return m_name == dict.m_name && m_plugin == dict.m_plugin; }

            private:
                QString m_plugin;
                QString m_name;
        };

        /**
         * Construct the DictionaryManager.
         */
        DictionaryManager(QObject *parent = 0);

        /**
         * Destructor.
         */
        ~DictionaryManager();

        /**
         * Save settings.
         */
        void saveSettings();

    private:

        /**
         * Load settings.
         */
        void loadSettings();

        class Private;
        Private *const d;
};

}

#endif // MULA_CORE_DICTIONARYMANAGER_H

