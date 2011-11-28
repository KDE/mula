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
#include "singleton.h"

#include <QtCore/QStringList>
#include <QtCore/QMultiHash>

namespace MulaCore
{
    class DictionaryManager: public MulaCore::Singleton<DictionaryManager>
    {
        Q_OBJECT
        MULA_SINGLETON( DictionaryManager )

        public:
            /**
             * Returns true if word exists in dictionaries,
             * otherwise false.
             *
             * @see translate
             */
            bool isTranslatable(const QString &word);

            /**
             * Returns translation for word. If word not found, returns
             * "Not found!"
             *
             * @param word The word for translation
             *
             * @see isTranslatable
             */
            QString translate(const QString &word);

            /**
             * Returns a list of similar words contained in dictionaries.
             *
             * @param word The word for translation
             *
             * @return List of similar words
             */
            QStringList findSimilarWords(const QString &word);

            /**
             * Returns a list of available dictionaries.
             * The first item in pair is a plugin name and the second item
             * is a dictionary name.
             */
            QMultiHash<QString, QString> availableDictionaryList() const;

            /**
             * Returns a list of loaded dictionaries.
             * The first item in pair is a plugin name and the second item
             * is a dictionary name.
             *
             * @see setLoadedDictionaryList, availableDictionaryList
             */
            QMultiHash<QString, QString> loadedDictionaryList() const;

            /**
             * Sets a list of the loaded dictionaries.
             * The first item in pair is a plugin name and the second item
             * is a dictionary name.
             * If dictionary cannot be loaded it will not be added to
             * availableDicts list.
             *
             * @param loadedDictionaryList List of the loaded dictionaries
             * 
             * @see loadedDictionaryList, availableDictionaryList
             */
            void setLoadedDictionaryList(const QMultiHash<QString, QString> &loadedDictionaryList);

            /**
             * Reloads the loaded dictionaries
             */
            void reloadDictionaryList();

            /**
             * Saves the dictionary settings
             *
             * @see loadDictionarySettings
             */
            void saveDictionarySettings();

        private:
            /**
             * Destructor
             */
            ~DictionaryManager();

            /**
             * Loads settings
             *
             * @see saveDictionarySettings
             */
            void loadDictionarySettings();

            class Private;
            Private *const d;
    };
}

#endif // MULA_CORE_DICTIONARYMANAGER_H

