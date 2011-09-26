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
             * Returns true if word exists in dictionaries of the plugins,
             * otherwise false.
             *
             * @param word The desired word to look up in the dictionaries of the
             * plugins
             * @return Whether the translation exists in the the dictionaries of
             * the plugins
             * @see translate
             */
            bool isTranslatable(const QString &word);

            /**
             * Returns the translation of the word from the dictionaries of the
             * plugins. If the word can not be found, the method returns "Not
             * found!"
             *
             * @param word The desired word to look up in the dictionaries of
             * the plugins
             * @return The translation of the desired word in the dictionaries
             * of the plugins
             * @see isTranslatable
             */
            QString translate(const QString &word);

            /**
             * Returns the list of the similar words contained in the dictionaries
             * of the plugins. If the word can not be found, the method returns
             * an empty list.
             *
             * @param word The desired word to look up in the dictionaries of
             * the plugins
             * @return The similar words in a list
             */
            QStringList findSimilarWords(const QString &word);

            /**
             * Returns the list of the available dictionaries.
             * The first item (key) is the plugin name, the second item (value) is
             * the dictionary name in the multihash.
             * Note: Plugins can have multi dictionaries, not just one, thus
             * that is why it is stored as a multihash
             *
             * @return The multihash containing the available plugin names and
             * the relevant dictionaries accordingly
             * @see loadedDictionaryList, setLoadedDictionaryList
             */
            QMultiHash<QString, QString> availableDictionaryList() const;

            /**
             * Returns the list of the loaded dictionaries.
             * The first item (key) is the plugin name, the second item is
             * the dictionary name in the multihash.
             * Note: Plugins can have multi dictionaries, not just one, thus
             * that is why it is stored as a multihash
             *
             * @return  The multihash containing the loaded plugin names and the
             * relevant dictionaries accordingly
             * @see availableDictionaryList, setLoadedDictionaryList
             */
            const QMultiHash<QString, QString> &loadedDictionaryList() const;

            /**
             * Sets the list of the loaded dictionaries.
             * The first item (key) is the plugin name, the second item is
             * the dictionary name in the multihash.
             * Note: Plugins can have multi dictionaries, not just one, thus
             * that is why it is stored as a multihash
             *
             * @param loadedDictionaryList The multihash containing the loaded
             * plugin names and the relevant dictionaries accordingly
             * @see availableDictionaryList, loadedDictionaryList
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
             * Loads the dictionary settings
             *
             * @see saveDictionarySettings
             */
            void loadDictionarySettings();

            class Private;
            Private *const d;
    };
}

#endif // MULA_CORE_DICTIONARYMANAGER_H

