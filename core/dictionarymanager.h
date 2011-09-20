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

#ifndef MULA_CORE_DICTIONARYMANAGER_H
#define MULA_CORE_DICTIONARYMANAGER_H

#include "dictionaryplugin.h"
#include "dictionarydataitem.h"
#include "singleton.h"

#include <QtCore/QStringList>
#include <QtCore/QPair>
#include <QtCore/QHash>
#include <QtCore/QPluginLoader>

namespace MulaCore
{
    class DictionaryManager: public MulaCore::Singleton<DictionaryManager>
    {
        Q_OBJECT
        MULA_SINGLETON( DictionaryManager )

        public:
            /**
             * Saves the dictionary settings.
             */
            void saveDictionarySettings();

            /** 
             * Returns true if word exists in dictionaries,
             * otherwise false.
             */
            bool isTranslatable(const QString &word);

            /** 
             * Returns translation for word. If word not found, returns
             * "Not found!"
             */
            QString translate(const QString &word);

            /** 
             * Returns a list of similar words contained in dictionaries.
             */
            QStringList findSimilarWords(const QString &word);

            /** 
             * Returns a list of available dictionaries.
             * The first item in pair is a plugin name, the second item
             * in pair is a dictionary name.
             */
            QList<DictionaryDataItem> availableDictionaries() const;

            /** 
             * Returns a list of loaded dictionaries. 
             * The first item in pair is a plugin name, the second item
             * in pair is a dictionary name.
             */
            const QList<DictionaryDataItem> &loadedDictionaries() const;

            /** 
             * Sets a loaded dictionaries.
             * The first item in pair is a plugin name, the second item
             * in pair is a dictionary name.
             * If dictionary cannot be loaded it will not be added to 
             * availableDicts list.
             */
            void setLoadedDictionaries(const QList<DictionaryDataItem> &loadedDictionaries);

            /**
             * Reloads the loaded dictionaries.
             */
            void reloadDictionaries();

        private:
            /**
             * Destructor.
             */
            ~DictionaryManager();

            /**
             * Load settings.
             */
            void loadDictionarySettings();

            class Private;
            Private *const d;
    };
}

#endif // MULA_CORE_DICTIONARYMANAGER_H

